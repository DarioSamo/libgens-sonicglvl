//
// Generated by Microsoft (R) HLSL Shader Compiler 9.29.952.3111
//
// Parameters:
//
//   float4 g_Ambient;
//   float4 g_Diffuse;
//   float4 g_EyePosition;
//   sampler2D g_GISampler;
//   float4 g_PowerGlossLevel;
//   float4 g_Specular;
//   float4 mrgGlobalLight_Diffuse;
//   float4 mrgGlobalLight_Direction;
//   float4 mrgGlobalLight_Specular;
//   sampler2D sampDLScatter;
//   sampler2D sampDif0;
//   sampler2D sampDif1;
//   sampler2D sampSpe0;
//   sampler2D sampSpe1;
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
//   mrgGlobalLight_Diffuse   c36      1
//   mrgGlobalLight_Specular  c37      1
//   sampDif0                 s0       1
//   sampDif1                 s1       1
//   sampSpe0                 s2       1
//   sampSpe1                 s3       1
//   sampDLScatter            s9       1
//   g_GISampler              s10      1
//

    ps_3_0
    def c0, 10, 4, 0, 0
    dcl_texcoord v0
    dcl_texcoord1 v1.xy
    dcl_texcoord2 v2.xyz
    dcl_texcoord3_pp v3
    dcl_texcoord5 v4.xy
    dcl_color_pp v5
    dcl_2d s0
    dcl_2d s1
    dcl_2d s2
    dcl_2d s3
    dcl_2d s9
    dcl_2d s10
    texld_pp r0, v0, s0
    texld_pp r1, v1, s1
    lrp_pp r2, v5.w, r0, r1
    mul_pp oC0.w, r2.w, c16.w
    mov r0.y, c20.y
    mul_pp r0.x, r0.y, c0.x
    exp_pp r0.x, r0.x
    mul_pp r0.x, r0.x, c0.y
    add_pp r0.yzw, c22.xxyz, -v2.xxyz
    dp3_pp r1.x, r0.yzww, r0.yzww
    rsq_pp r1.x, r1.x
    mad_pp r0.yzw, r0, r1.x, -c10.xxyz
    nrm_pp r1.xyz, r0.yzww
    nrm_pp r3.xyz, v3
    dp3_sat_pp r0.y, r1, r3
    dp3_sat_pp r0.z, r3, -c10
    pow_pp r1.x, r0.y, r0.x
    texld_pp r3, v0.zwzw, s10
    mul_pp r0.x, r3.w, c20.z
    mul_pp r0.x, r1.x, r0.x
    mul_pp r0.xyw, r0.x, c37.xyzz
    mul_pp r0.xyw, r0, c18.xyzz
    texld_pp r1, v0, s2
    texld_pp r4, v1, s3
    lrp_pp r5.xyz, v5.w, r1, r4
    mul_pp r0.xyw, r0, r5.xyzz
    mul_pp r0.xyw, r0, v3.w
    mul r0.z, r0.z, r3.w
    mul_pp r1.xyz, r3, r3
    mul_pp r3.xyz, r0.z, c36
    mad_pp r1.xyz, r1, c17, r3
    mul_pp r1.xyz, r1, c16
    mad_pp r0.xyz, r1, r2, r0.xyww
    mul_pp r0.xyz, r0, v5
    texld_pp r1, v4.yxzw, s9
    mad_pp oC0.xyz, r0, r1.w, r1

// approximately 42 instruction slots used (6 texture, 36 arithmetic)
