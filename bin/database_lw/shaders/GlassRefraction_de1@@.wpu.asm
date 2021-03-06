//
// Generated by Microsoft (R) HLSL Shader Compiler 9.29.952.3111
//
// Parameters:
//
//   float4 g_Ambient;
//   float4 g_Diffuse;
//   float4 g_EyePosition;
//   sampler2D g_FramebufferSampler;
//   float4 g_GIModeParam;
//   sampler2D g_GISampler;
//   row_major float4x4 g_MtxView;
//   float4 g_PowerGlossLevel;
//   float4 g_Specular;
//   float4 g_ViewportSize;
//   float4 mrgEyeLight_Diffuse;
//   float4 mrgEyeLight_Range;
//   float4 mrgEyeLight_Specular;
//   float2 mrgFresnelParam;
//   float2 mrgGlassRefractionParam;
//   float4 mrgGlobalLight_Diffuse;
//   float4 mrgGlobalLight_Direction;
//   float4 mrgGlobalLight_Specular;
//   float4 mrgInShadowScale;
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
//   float4 mrgLuminanceRange;
//   sampler2D sampDLScatter;
//   sampler2D sampDif;
//   sampler2D sampEnv;
//
//
// Registers:
//
//   Name                     Reg   Size
//   ------------------------ ----- ----
//   mrgGlobalLight_Direction c10      1
//   g_Diffuse                c16      1
//   g_Ambient                c17      1
//   g_Specular               c18      1
//   g_PowerGlossLevel        c20      1
//   g_EyePosition            c22      1
//   g_ViewportSize           c24      1
//   mrgGlobalLight_Diffuse   c36      1
//   mrgGlobalLight_Specular  c37      1
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
//   mrgEyeLight_Specular     c59      1
//   mrgEyeLight_Range        c60      1
//   mrgFresnelParam          c62      1
//   mrgLuminanceRange        c63      1
//   mrgInShadowScale         c64      1
//   g_GIModeParam            c69      1
//   g_MtxView                c90      3
//   mrgGlassRefractionParam  c150     1
//   sampDif                  s0       1
//   sampEnv                  s1       1
//   sampDLScatter            s9       1
//   g_GISampler              s10      1
//   g_FramebufferSampler     s11      1
//

    ps_3_0
    def c0, 2, -2, 10, 4
    def c1, 0.5, 1, 0, -1
    def c2, 0.5, -0.5, -0.707106769, 0
    dcl_texcoord v0
    dcl_texcoord2 v1.xyz
    dcl_texcoord3 v2.xyz
    dcl_texcoord5 v3.xy
    dcl vPos.xy
    dcl_2d s0
    dcl_2d s1
    dcl_2d s9
    dcl_2d s10
    dcl_2d s11
    add_pp r0.xyz, -c54, v1
    dp3_pp r0.w, r0, r0
    rsq_pp r0.w, r0.w
    rcp_pp r1.x, r0.w
    mul_pp r0.xyz, r0.w, r0
    mad_sat_pp r0.w, -r1.x, c56.z, c56.w
    mul r1.xyz, r0.w, c55
    add_pp r2.xyz, -c50, v1
    dp3_pp r0.w, r2, r2
    rsq_pp r0.w, r0.w
    rcp_pp r1.w, r0.w
    mul_pp r2.xyz, r0.w, r2
    mad_sat_pp r0.w, -r1.w, c52.z, c52.w
    mul r3.xyz, r0.w, c51
    add_pp r4.xyz, -c42, v1
    dp3_pp r0.w, r4, r4
    rsq_pp r0.w, r0.w
    rcp_pp r1.w, r0.w
    mul_pp r4.xyz, r0.w, r4
    mad_sat_pp r0.w, -r1.w, c44.z, c44.w
    mul r5.xyz, r0.w, c43
    add_pp r6.xyz, -c46, v1
    dp3_pp r0.w, r6, r6
    rsq_pp r0.w, r0.w
    rcp_pp r1.w, r0.w
    mul_pp r6.xyz, r0.w, r6
    mad_sat_pp r0.w, -r1.w, c48.z, c48.w
    mul r7.xyz, r0.w, c47
    mov r8.y, c20.y
    mul_pp r0.w, r8.y, c0.z
    exp_pp r0.w, r0.w
    mul_pp r0.w, r0.w, c0.w
    add_pp r8.xyz, c22, -v1
    dp3_pp r1.w, r8, r8
    rsq_pp r1.w, r1.w
    mad_pp r9.xyz, r8, r1.w, -r6
    nrm_pp r10.xyz, r9
    nrm_pp r9.xyz, v2
    dp3_sat_pp r2.w, r10, r9
    pow_pp r3.w, r2.w, r0.w
    mul_pp r2.w, r3.w, c20.z
    mul_pp r10.xyz, r7, r2.w
    mad_pp r11.xyz, r8, r1.w, -r4
    dp3_sat_pp r2.w, r9, -r4
    nrm_pp r4.xyz, r11
    dp3_sat_pp r3.w, r4, r9
    pow_pp r4.x, r3.w, r0.w
    mul_pp r3.w, r4.x, c20.z
    mad_pp r4.xyz, r3.w, r5, r10
    mad_pp r10.xyz, r8, r1.w, -r2
    dp3_sat_pp r2.x, r9, -r2
    nrm_pp r11.xyz, r10
    dp3_sat_pp r2.y, r11, r9
    pow_pp r3.w, r2.y, r0.w
    mul_pp r2.y, r3.w, c20.z
    mad_pp r4.xyz, r2.y, r3, r4
    mad_pp r10.xyz, r8, r1.w, -r0
    dp3_sat_pp r0.x, r9, -r0
    nrm_pp r11.xyz, r10
    dp3_sat_pp r0.y, r11, r9
    pow_pp r2.y, r0.y, r0.w
    mul_pp r0.y, r2.y, c20.z
    mad_pp r4.xyz, r0.y, r1, r4
    mad_pp r10.xyz, r8, r1.w, -c10
    mul_pp r8.xyz, r1.w, r8
    rcp r0.y, r1.w
    mad_sat_pp r0.y, -r0.y, c60.z, c60.w
    nrm_pp r11.xyz, r10
    dp3_sat_pp r0.z, r11, r9
    pow_pp r1.w, r0.z, r0.w
    texld r10, v0.zwzw, s10
    mul_pp r0.z, r10.w, c20.z
    mul_pp r0.z, r1.w, r0.z
    mad_pp r4.xyz, r0.z, c37, r4
    mul_pp r11.xyz, r0.y, c59
    mul r12.xyz, r0.y, c58
    add_pp r13.xyz, r8, r8
    dp3_sat_pp r0.y, r9, r8
    nrm_pp r8.xyz, r13
    dp3_sat_pp r0.z, r8, r9
    pow_pp r1.w, r0.z, r0.w
    mul_pp r0.z, r1.w, c20.z
    mad_pp r4.xyz, r0.z, r11, r4
    mul r8, r9.y, c91
    mad r8, r9.x, c90, r8
    mad r8, r9.z, c92, r8
    dp4 r0.z, r8, r8
    rsq r0.z, r0.z
    mul r0.zw, r0.z, r8.xyxy
    mad_pp r0.zw, r0, c2.xyxy, c2.x
    texld_pp r11, r0.zwzw, s1
    mul r0.z, c64.z, c64.x
    mov r13.xz, c64
    mad r0.w, r13.x, -r13.z, r13.z
    mad_pp r0.z, r10.w, r0.w, r0.z
    texld_pp r13, v0, s0
    mov r14.yzw, c1
    mad r0.w, c16.w, -r13.w, r14.y
    mul r0.w, r0.w, c63.x
    mul r0.z, r0.z, r0.w
    mul_pp r11.xyz, r0.z, r11
    mad_pp r4.xyz, r4, c18, r11
    dp3_sat_pp r0.z, r9, -r6
    dp3_sat_pp r0.w, r9, -c10
    mul r0.w, r0.w, r10.w
    mul_pp r6.xyz, r10, r10
    mul_pp r7.xyz, r7, r0.z
    mad_pp r2.yzw, r2.w, r5.xxyz, r7.xxyz
    mad_pp r2.xyz, r2.x, r3, r2.yzww
    mad_pp r1.xyz, r0.x, r1, r2
    mad_pp r0.xzw, r0.w, c36.xyyz, r1.xyyz
    mad_pp r0.xzw, r0.y, r12.xyyz, r0
    add_pp r0.y, -r0.y, c1.y
    mad_pp r0.xzw, r6.xyyz, c17.xyyz, r0
    mul_pp r0.xzw, r0, c16.xyyz
    mad r1.x, r8.z, -r8.z, c1.y
    mul r1.y, c150.x, c150.x
    mad r1.x, r1.y, -r1.x, c1.y
    cmp r1.y, r1.x, c1.y, c1.z
    mul r1.x, r1.y, r1.x
    mul r1.y, r1.y, c150.x
    rsq r1.x, r1.x
    rcp r1.x, r1.x
    mad r1.x, r1.y, -r8.z, r1.x
    mul_pp r1.xy, r8, r1.x
    add r1.zw, c1.x, vPos.xyxy
    mul r1.zw, r1, c24
    mad r2, c24.zzww, r14.yzzw, r14.zywz
    mad r2.xy, r1.zwzw, c0, r2
    texld r3, r1.zwzw, s11
    add_pp r1.zw, r2, r2.xyxy
    mad_pp r1.xy, r1, c2.z, r1.zwzw
    mad r1.xy, r1, c2, c2.x
    texld_pp r1, r1, s11
    mad r0.xzw, r0, r13.xyyz, -r1.xyyz
    mul_pp r1.w, r13.w, c16.w
    mad_pp r0.xzw, r1.w, r0, r1.xyyz
    mul_pp r1.x, r0.y, r0.y
    mul_pp r1.x, r1.x, r1.x
    mul_pp r0.y, r0.y, r1.x
    lrp_pp r1.x, r0.y, r14.y, c62.x
    mad_pp r0.xyz, r1.x, r4, r0.xzww
    texld_pp r1, v3.yxzw, s9
    mad_pp r0.xyz, r0, r1.w, r1
    add r1.xyz, -r0, r3
    mul r0.w, c69.z, v3.x
    mad_pp oC0.xyz, r0.w, r1, r0
    mov_pp oC0.w, c1.y

// approximately 174 instruction slots used (6 texture, 168 arithmetic)
