module DirectLink1270(input[32:0] in, output [41:0] out);
	assign out[32:0]=~in[32:0];
	assign out[41:33]=~in[8:0];
endmodule