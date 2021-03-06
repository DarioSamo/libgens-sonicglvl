//
// Generated by Microsoft (R) HLSL Shader Compiler 9.29.952.3111
//
// Parameters:
//
//   float4 g_Ambient;
//   float4 g_Diffuse;
//   float4 g_EyePosition;
//   float4 g_aLightField[6];
//   float4 mrgEyeLight_Diffuse;
//   float4 mrgEyeLight_Range;
//   float4 mrgGlobalLight_Diffuse;
//   float4 mrgGlobalLight_Direction;
//   float4 mrgLocalLight1_Color;
//   float4 mrgLocalLight1_Position;
//   float4 mrgLocalLight1_Range;
//   float4 mrgLocalLight2_Color;
//   float4 mrgLocalLight2_Position;
//   float4 mrgLocalLight2_Range;
//   float4 mrgLocalLight3_Color;
//   float4 mrgLocalLight3_Position;
//   float4 mrgLocalLight3_Range;
//   float4 mrgLocalLight4_Color;
//   float4 mrgLocalLight4_Position;
//   float4 mrgLocalLight4_Range;
//   sampler2D s0;
//   sampler2D s1;
//   sampler2D s2;
//
//
// Registers:
//
//   Name                     Reg   Size
//   ------------------------ ----- ----
//   mrgGlobalLight_Direction c10      1
//   g_Diffuse                c16      1
//   g_Ambient                c17      1
//   g_EyePosition            c22      1
//   mrgGlobalLight_Diffuse   c36      1
//   mrgLocalLight1_Position  c42      1
//   mrgLocalLight1_Color     c43      1
//   mrgLocalLight1_Range     c44      1
//   mrgLocalLight2_Position  c46      1
//   mrgLocalLight2_Color     c47      1
//   mrgLocalLight2_Range     c48      1
//   mrgLocalLight3_Position  c50      1
//   mrgLocalLight3_Color     c51      1
//   mrgLocalLight3_Range     c52      1
//   mrgLocalLight4_Position  c54      1
//   mrgLocalLight4_Color     c55      1
//   mrgLocalLight4_Range     c56      1
//   mrgEyeLight_Diffuse      c58      1
//   mrgEyeLight_Range        c60      1
//   g_aLightField            c77      1
//   s0                       s0       1
//   s1                       s1       1
//   s2                       s2       1
//

    ps_3_0
    def c0, 1, 0, 0, 0
    dcl_texcoord v0
    dcl_texcoord6 v1.xyz
    dcl_color_pp v2.xyz
    dcl_2d s0
    dcl_2d s1
    dcl_2d s2
    add_pp r0.xyz, -c42, v1
    dp3_pp r0.w, r0, r0
    rsq_pp r0.w, r0.w
    rcp_pp r1.x, r0.w
    mul_pp r0.xyz, r0.w, r0
    mad_sat_pp r0.w, -r1.x, c44.z, c44.w
    mul r1.xyz, r0.w, c43
    add_pp r2.xyz, -c46, v1
    dp3_pp r0.w, r2, r2
    rsq_pp r0.w, r0.w
    rcp_pp r1.w, r0.w
    mul_pp r2.xyz, r0.w, r2
    mad_sat_pp r0.w, -r1.w, c48.z, c48.w
    mul r3.xyz, r0.w, c47
    nrm_pp r4.xyz, v2
    dp3_sat_pp r0.w, r4, -r2
    mul_pp r2.xyz, r3, r0.w
    dp3_sat_pp r0.x, r4, -r0
    mad_pp r0.xyz, r0.x, r1, r2
    add_pp r1.xyz, -c50, v1
    dp3_pp r0.w, r1, r1
    rsq_pp r0.w, r0.w
    rcp_pp r1.w, r0.w
    mul_pp r1.xyz, r0.w, r1
    dp3_sat_pp r0.w, r4, -r1
    mad_sat_pp r1.x, -r1.w, c52.z, c52.w
    mul r1.xyz, r1.x, c51
    mad_pp r0.xyz, r0.w, r1, r0
    add_pp r1.xyz, -c54, v1
    dp3_pp r0.w, r1, r1
    rsq_pp r0.w, r0.w
    rcp_pp r1.w, r0.w
    mul_pp r1.xyz, r0.w, r1
    dp3_sat_pp r0.w, r4, -r1
    mad_sat_pp r1.x, -r1.w, c56.z, c56.w
    mul r1.xyz, r1.x, c55
    mad_pp r0.xyz, r0.w, r1, r0
    dp3_sat_pp r0.w, r4, -c10
    mul r1, c0.xxyy, v0.zwzz
    texldl_pp r2, r1, s2
    texldl_pp r1, r1, s1
    mul r2.w, r2.w, c77.w
    mul_pp r2.xyz, r2, r2
    mul r0.w, r0.w, r2.w
    mad_pp r0.xyz, r0.w, c36, r0
    add r3.xyz, c22, -v1
    dp3 r0.w, r3, r3
    rsq r0.w, r0.w
    rcp r2.w, r0.w
    mul_pp r3.xyz, r0.w, r3
    dp3_sat_pp r0.w, r4, r3
    mad_sat_pp r2.w, -r2.w, c60.z, c60.w
    mul r3.xyz, r2.w, c58
    mad_pp r0.xyz, r0.w, r3, r0
    mad_pp r0.xyz, r2, c17, r0
    mul_pp r1.xyz, r0, r1
    texld_pp r0, v0, s0
    mul_pp r0, r0, r1
    mul_pp oC0, r0, c16

// approximately 63 instruction slots used (5 texture, 58 arithmetic)
