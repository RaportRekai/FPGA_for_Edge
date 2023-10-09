// Top Module to drive the OLED
/*
Inputs
clock - 100MHz Clock
reset - reset push button
outputs - OLED Interface Ports
*/

// Module Declaration
module oled_main(
    input  clock, reset,
    output oled_spi_clk, oled_spi_data, oled_vdd,
    output oled_vbat, oled_reset_n, oled_dc_n
);

// Text to Display in oled is initialized

localparam  myString1 = " eYSIP22 Intern ",
            myString2 = "53 FPGA for Edge",
            myString3 = "Anupam Dan Vicky",
            myString4 = " Isha and Lohit ";

localparam StringLen = 16;
// Parameters for State Machine are declared
parameter IDLE = 'd0, SEND = 'd1, DONE = 'd2;

integer byteCounter;
reg [7:0] sendData;
reg [2:0] switchline;
reg [1:0] state;
reg sendDataValid;
wire sendDone;
 
always @(posedge clock) begin
    if(reset) begin
        state <= IDLE; byteCounter <= StringLen; sendDataValid <= 1'b0;
    end
    else begin
        case(state)
            IDLE:begin
                if(!sendDone) begin
                    if(switchline == 0) sendData <= myString1[(byteCounter*8-1)-:8];
                    if(switchline == 1) sendData <= myString2[(byteCounter*8-1)-:8];
                    if(switchline == 2) sendData <= myString3[(byteCounter*8-1)-:8];
                    if(switchline == 3) sendData <= myString4[(byteCounter*8-1)-:8];
                    sendDataValid <= 1'b1; state <= SEND;
                end
            end
            SEND:begin
                if(sendDone) begin
                    sendDataValid <= 1'b0; byteCounter <= byteCounter-1;
                    if(byteCounter != 1) state <= IDLE;
                    else begin
                        state <= DONE; byteCounter <= StringLen;
                        switchline <= switchline + 1'b1;
                    end
                end
            end
            DONE:begin
                if(switchline == 4) state <= DONE;
                else state <= IDLE;
            end
        endcase
    end
end
    
    
oled_controller OC(
    .clock(clock), .reset(reset), .oled_spi_clk(oled_spi_clk), .oled_spi_data(oled_spi_data),
    .oled_vdd(oled_vdd), .oled_vbat(oled_vbat), .oled_reset_n(oled_reset_n),
    .oled_dc_n(oled_dc_n), .sdin(sendData), .d_valid(sendDataValid), .txDone(sendDone)
);    
    
endmodule
