// Delay Generator
/*
Inputs
clock    - 100MHz clock
en_delay - Enable Delay Generator
Outputs
delay_done - Created Delay sucessfully
*/
// Module Declaration
module delay_generator(
    input clock,
    input en_delay,
    output reg delay_done
);
    
reg [17:0] counter = 0;

always @(posedge clock) begin
    if(en_delay & counter!=200000) counter <= counter+1;
    else counter <= 0;
end

always @(posedge clock) begin
    if(en_delay & counter==200000) delay_done <= 1'b1;
    else delay_done <= 1'b0;
end
    
endmodule
