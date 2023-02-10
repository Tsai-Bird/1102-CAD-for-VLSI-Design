`timescale 1ns/1ps 
module c17 (gat1, gat2, gat3, gat6, gat7, gat_out22, gat_out23);
input gat1, gat2, gat3, gat6, gat7;
output gat_out22, gat_out23;
wire gat_out10, gat_out11, gat_out16, gat_out19;
nand gat10 (gat_out10, gat1, gat3);
nand gat11 (gat_out11, gat3, gat6);
nand gat16 (gat_out16, gat2, gat_out11);
nand gat19 (gat_out19, gat_out11, gat7);
nand gat22 (gat_out22, gat_out10, gat_out16);
nand gat23 (gat_out23, gat_out16, gat_out19);
endmodule