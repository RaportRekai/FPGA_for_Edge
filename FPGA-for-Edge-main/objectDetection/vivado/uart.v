// Module Declaration
module uart(
    input clk, reset, txStart, result,
    input [2:0] imageID,
    input [3:0] acc_1, acc_2, acc_3,
    input [3:0] bb1_1, bb1_2, bb1_3, bb1_4, bb1_5, bb1_6,
    input [3:0] bb2_1, bb2_2, bb2_3, bb2_4, bb2_5, bb2_6,
    output reg txOut, txDone
);

// UART Transmission Parameters
/*
baud rate = 115200
clocks per bit (cpb) = (100*10^6)/115200 = 868
start bit = 0, stop bit = 1;
1 start bit, 1 stop bit and no parity bit
*/
parameter cpb = 868;

// State Machine Parameters
parameter   idle = 2'b00, start = 2'b01,
            send = 2'b10, stop = 2'b11;

// ASCII Values for the Data that need to be sent.
parameter   c0 = 8'b00110000, c1 = 8'b00110001, 
            c2 = 8'b00110010, c3 = 8'b00110011,
            c4 = 8'b00110100, c5 = 8'b00110101,
            c6 = 8'b00110110, c7 = 8'b00110111,
            c8 = 8'b00111000, c9 = 8'b00111001,
            cHash = 8'b00100011, cDash = 8'b00101101,
            cDot = 8'b00101110, cLine = 8'b00001010;

// Variables
reg [9:0] counter;
reg [7:0] msg;
reg [4:0] dataIndex, imgid, tresult;
reg [3:0] track;
reg [2:0] bitIndex;
reg [1:0] state;

// Initializing Variables
initial begin
    counter = 0; msg = 0; dataIndex = 0; imgid = 0; txDone = 0;
    track = 0; state = 0; bitIndex = 0; tresult = 0; txOut = 0;
end

// Function asciiChooser
// This function is used to choose the ascii value for the given
// input number using a simple case structure
function [7:0] asciiChooser(input [3:0] number); begin
    case(number)
        0: asciiChooser = c0;
        1: asciiChooser = c1;
        2: asciiChooser = c2;
        3: asciiChooser = c3;
        4: asciiChooser = c4;
        5: asciiChooser = c5;
        6: asciiChooser = c6;
        7: asciiChooser = c7;
        8: asciiChooser = c8;
        9: asciiChooser = c9;
    endcase
end
endfunction

always @(posedge clk) begin
    // Transmission starts only if txStart is High and txDone is Low
    if(reset) begin
        counter = 0; msg = 0; dataIndex = 0; imgid = 0; txDone = 0;
        track = 0; state = 0; bitIndex = 0; tresult = 0; txOut = 0;
    end
    else if(txStart && !txDone) begin
        counter = counter + 1'b1;
        case(state)
            // IDLE Bit
            idle: begin
                counter = counter + 1'b1;
                if (counter < cpb) txOut = 1;
                else begin
                    counter = 0; state = start;
                end
                tresult = result; imgid = imageID;
            end
            // Start Bit
            start: begin
                if(counter < cpb) txOut = 0;
                else if(counter > cpb-1) begin
                    counter = 0; state = send;
                end
            end
            // Sending Message
            // msg Format = #2-1-97.8-213-231-345-322#
            send: begin
                case(dataIndex)
                    0:  msg = cHash;
                    1:  msg = asciiChooser(imageID); // Image Id
                    2:  msg = cDash;
                    3:  msg = asciiChooser(result); // Result
                    4:  msg = cDash;
                    5:  msg = asciiChooser(acc_1); // Accuracy
                    6:  msg = asciiChooser(acc_2);
                    7:  msg = cDot;
                    8:  msg = asciiChooser(acc_3);
                    9:  msg = cDash;
                    10: msg = asciiChooser(bb1_1); // BB x1
                    11: msg = asciiChooser(bb1_2);
                    12: msg = asciiChooser(bb1_3);
                    13: msg = cDash;
                    14: msg = asciiChooser(bb1_4); // BB x2
                    15: msg = asciiChooser(bb1_5);
                    16: msg = asciiChooser(bb1_6);
                    17: msg = cDash;
                    18: msg = asciiChooser(bb2_1); // BB y1
                    19: msg = asciiChooser(bb2_2);
                    20: msg = asciiChooser(bb2_3);
                    21: msg = cDash;
                    22: msg = asciiChooser(bb2_4); // BB y2
                    23: msg = asciiChooser(bb2_5);
                    24: msg = asciiChooser(bb2_6);
                    25: msg = cHash;
                    26: msg = cLine;
                    default: msg = cHash;
                endcase
                // msg transmission
                if(counter < cpb) txOut = msg[bitIndex];
                else if(bitIndex < 8) begin
                    counter = 0;
                    if(bitIndex != 7) bitIndex = bitIndex + 1'b1;
                    else begin
                        counter = 0; bitIndex = 0; state = stop;
                        dataIndex = dataIndex + 1'b1;
                    end
                    if(dataIndex == 27) begin
                        dataIndex = 0; txDone = 1;
                    end
                end
            end
            // Stop Bit
            stop: begin
                if(counter < cpb) txOut = 1;
                else begin
                    counter = 0; state = idle;
                end
            end
        endcase
    end
end

endmodule