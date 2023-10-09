// Module Creation
module uart_tx(
    input clock, reset, result, compute,
    input [2:0] imgID,
    input [9:0] accuracy,
    input [8:0] bbx1, bby1, bbx2, bby2,
    output tx, txComplete
);

parameter init = 2'b00, choose = 2'b01, convert = 2'b10, done = 2'b11;

reg [9:0] no = 15;
reg [3:0] acc1, acc2, acc3, trackVar, mod;
reg [3:0] bb11, bb12, bb13, bb14, bb15, bb16;
reg [3:0] bb21, bb22, bb23, bb24, bb25, bb26;
reg [2:0] counter, track;
reg [1:0] state = init;
reg txTransmit;

// This block converts a decimal value into digits
always @(posedge clock) begin
    if(reset) state = init;
    else if(compute) begin
        case(state)
            init: begin
                no = 15; acc1 = 0; acc2 = 0; acc3 = 0; trackVar = 0; mod = 0;
                bb11 = 0; bb12 = 0; bb13 = 0; bb14 = 0; bb15 = 0; bb16 = 0;
                bb21 = 0; bb22 = 0; bb23 = 0; bb24 = 0; bb25 = 0; bb26 = 0;
                counter = 0; state = choose; track = 0; txTransmit = 0;
            end
            choose: begin
                case(track)
                    0: no = accuracy;
                    1: no = bbx1;
                    2: no = bby1;
                    3: no = bbx2;
                    4: no = bby2;
                    default: state = done;
                endcase
                if(track <= 4) state = convert;
                else state = done;
            end
            convert: begin
                if(counter != 3) begin
                    mod = no%10; no = no/10; counter = counter + 1'b1;
                    case(trackVar)
                        0:  acc1 = mod;
                        1:  acc2 = mod;
                        2:  acc3 = mod;
                        3:  bb11 = mod; 
                        4:  bb12 = mod;
                        5:  bb13 = mod;
                        6:  bb14 = mod;
                        7:  bb15 = mod;
                        8:  bb16 = mod;
                        9:  bb21 = mod;
                        10: bb22 = mod;
                        11: bb23 = mod;
                        12: bb24 = mod;
                        13: bb25 = mod;
                        14: bb26 = mod;
                        default: state = done;
                    endcase
                    trackVar = trackVar + 1'b1;
                end
                else begin
                    counter = 0; track = track + 1'b1; state = choose;
                end
            end
            done: begin
                txTransmit = 1;
            end
        endcase
    end
end

wire txOut, txDone;

// UART Module Instantiation
uart uart_inst(
    clock, reset, txTransmit,
    result, imgID, acc3, acc2, acc1,
    bb13, bb12, bb11, bb16, bb15, bb14,
    bb23, bb22, bb21, bb26, bb25, bb24,
    txOut, txDone
);

assign tx = txOut;
assign txComplete = txDone;

endmodule
