// SPI Controller for OLED Interface
/*
Inputs
clock - 100MHz clock from on board Oscillator
reset - reset push button
din   - 8-bit data input to the spi controller

Outputs
msg_done - data transmission completed
spi_data - channel to send data
spi_clk - 10Mhz clock for OLED operations
*/

// Module Declaration
module spi_controller(
    input  clock, reset, load_data,
    input [7:0] din,
    output reg msg_done, spi_data,
    output spi_clk
);

// Defining parameters for State Machine
parameter IDLE = 'd0, SEND = 'd1, DONE = 'd2;

reg [7:0] shift_reg;
reg [2:0] counter=0, dataCount;
reg [1:0] state;
reg sd_clk, ce;

// Mapping registers to output
assign spi_clk = (ce == 1) ? sd_clk : 1'b1;

always @(posedge clock) begin
    if(counter != 4) counter <= counter + 1;
    else counter <= 0;
end

initial sd_clk <= 0;

// Frequency Scale Down
// 100MHz to 10MHz
always @(posedge clock) begin
    if(counter == 4) sd_clk <= ~sd_clk;
end

// SPI Protocol using FSM
always @(negedge sd_clk) begin
    if(reset) begin
        state <= IDLE; dataCount <= 0;
        msg_done <= 1'b0; ce <= 0; spi_data <= 1'b1;
    end
    else
    begin
        case(state)
            IDLE:begin
                if(load_data) begin
                    shift_reg <= din; state <= SEND; dataCount <= 0;
                end
            end
            SEND:begin
                // Sending the data MSB to LSB
                spi_data <= shift_reg[7]; ce <= 1;
                shift_reg <= {shift_reg[6:0],1'b0};
                if(dataCount != 7) dataCount <= dataCount + 1;
                else state <= DONE;
            end
            DONE:begin
                ce <= 0; msg_done <= 1'b1;
                if(!load_data) begin
                    msg_done <= 1'b0; state <= IDLE;
                end
            end
        endcase
    end
end
    
endmodule
