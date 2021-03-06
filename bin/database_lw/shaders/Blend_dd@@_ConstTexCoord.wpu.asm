//
// Generated by Microsoft (R) HLSL Shader Compiler 9.29.952.3111
//
// Parameters:
//
//   float4 g_Ambient;
//   float4 g_Diffuse;
//   sampler2D g_GISampler;
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
//   sampler2D sampDLScatter;
//   sampler2D sampDif0;
//   sampler2D sampDif1;
//
//
// Registers:
//
//   Name                     Reg   Size
//   ------------------------ ----- ----
//   mrgGlobalLight_Direction c10      1
//   g_Diffuse                c16      1
//   g_Ambient                c17      1
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
//   sampDif0                 s0       1
//   sampDif1                 s1       1
//   sampDLScatter            s9       1
//   g_GISampler              s10      1
//

    ps_3_0
    dcl_texcoord v0
    dcl_texcoord1 v1.xy
    dcl_texcoord2 v2.xyz
    dcl_texcoord3_pp v3.xyz
    dcl_texcoord5 v4.xy
    dcl_color_pp v5
    dcl_2d s0
    dcl_2d s1
    dcl_2d s9
    dcl_2d s10
    texld_pp r0, v0, s0
    texld_pp r1, v1, s1
    lrp_pp r2, v5.w, r0, r1
    mul_pp oC0.w, r2.w, c16.w
    add_pp r0.xyz, -c42, v2
    dp3_pp r0.w, r0, r0
    rsq_pp r0.w, r0.w
    rcp_pp r1.x, r0.w
    mul_pp r0.xyz, r0.w, r0
    mad_sat_pp r0.w, -r1.x, c44.z, c44.w
    mul r1.xyz, r0.w, c43
    add_pp r3.xyz, -c46, v2
    dp3_pp r0.w, r3, r3
    rsq_pp r0.w, r0.w
    rcp_pp r1.w, r0.w
    mul_pp r3.xyz, r0.w, r3
    mad_sat_pp r0.w, -r1.w, c48.z, c48.w
    mul r4.xyz, r0.w, c47
    nrm_pp r5.xyz, v3
    dp3_sat_pp r0.w, r5, -r3
    mul_pp r3.xyz, r4, r0.w
    dp3_sat_pp r0.x, r5, -r0
    mad_pp r0.xyz, r0.x, r1, r3
    add_pp r1.xyz, -c50, v2
    dp3_pp r0.w, r1, r1
    rsq_pp r0.w, r0.w
    rcp_pp r1.w, r0.w
    mul_pp r1.xyz, r0.w, r1
    dp3_sat_pp r0.w, r5, -r1
    mad_sat_pp r1.x, -r1.w, c52.z, c52.w
    mul r1.xyz, r1.x, c51
    mad_pp r0.xyz, r0.w, r1, r0
    add_pp r1.xyz, -c54, v2
    dp3_pp r0.w, r1, r1
    rsq_pp r0.w, r0.w
    rcp_pp r1.w, r0.w
    mul_pp r1.xyz, r0.w, r1
    dp3_sat_pp r0.w, r5, -r1
    dp3_sat_pp r1.x, r5, -c10
    mad_sat_pp r1.y, -r1.w, c56.z, c56.w
    mul r1.yzw, r1.y, c55.xxyz
    mad_pp r0.xyz, r0.w, r1.yzww, r0
    texld_pp r3, v0.zwzw, s10
    mul r0.w, r1.x, r3.w
    mul_pp r1.xyz, r3, r3
    mad_pp r0.xyz, r0.w, c36, r0
    mad_pp r0.xyz, r1, c17, r0
    mul_pp r0.xyz, r0, c16
    mul_pp r0.xyz, r2, r0
    mul_pp r0.xyz, r0, v5
    texld_pp r1, v4.yxzw, s9
    mad_pp oC0.xyz, r0, r1.w, r1

// approximately 54 instruction slots used (4 texture, 50 arithmetic)
