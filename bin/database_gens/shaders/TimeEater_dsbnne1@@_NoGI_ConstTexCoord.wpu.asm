//
// Generated by Microsoft (R) HLSL Shader Compiler 9.29.952.3111
//
// Parameters:
//
//   float4 g_Ambient;
//   float4 g_ChaosWaveParamEx;
//   float4 g_Diffuse;
//   float4 g_EyeDirection;
//   float4 g_EyePosition;
//   float4 g_ForceAlphaColor;
//   sampler2D g_FramebufferSampler;
//   float4 g_GI0Scale;
//   float4 g_LightScatteringColor;
//   row_major float4x4 g_MtxView;
//   float4 g_OpacityReflectionRefractionSpectype;
//   float4 g_PowerGlossLevel;
//   float4 g_ShadowMapParams;
//   sampler2D g_ShadowMapSampler;
//   float4 g_SonicSkinFalloffParam;
//   float4 g_Specular;
//   sampler2D g_VerticalShadowMapSampler;
//   float4 g_ViewportSize;
//   float4 g_aLightField[6];
//   float4 mrgEyeLight_Attribute;
//   float4 mrgEyeLight_Range;
//   float4 mrgEyeLight_Specular;
//   float4 mrgGlobalLight_Direction;
//   float4 mrgGlobalLight_Specular;
//   float4 mrgInShadowScale;
//   float4 mrgLocalLight0_Color;
//   float4 mrgLocalLight0_Position;
//   float4 mrgLocalLight0_Range;
//   float4 mrgLocalLight1_Color;
//   float4 mrgLocalLight1_Position;
//   float4 mrgLocalLight1_Range;
//   float4 mrgLocalLight2_Color;
//   float4 mrgLocalLight2_Position;
//   float4 mrgLocalLight2_Range;
//   float4 mrgLocalLight3_Color;
//   float4 mrgLocalLight3_Position;
//   float4 mrgLocalLight3_Range;
//   float4 mrgLuminanceRange;
//   sampler2D sampBlend;
//   sampler2D sampDif;
//   sampler2D sampEnv;
//   sampler2D sampNrm;
//   sampler2D sampNrm2;
//   sampler2D sampSpec;
//
//
// Registers:
//
//   Name                                  Reg   Size
//   ------------------------------------- ----- ----
//   mrgGlobalLight_Direction              c10      1
//   g_Diffuse                             c16      1
//   g_Ambient                             c17      1
//   g_Specular                            c18      1
//   g_PowerGlossLevel                     c20      1
//   g_OpacityReflectionRefractionSpectype c21      1
//   g_EyePosition                         c22      1
//   g_EyeDirection                        c23      1
//   g_ViewportSize                        c24      1
//   mrgGlobalLight_Specular               c37      1
//   mrgLocalLight0_Position               c38      1
//   mrgLocalLight0_Color                  c39      1
//   mrgLocalLight0_Range                  c40      1
//   mrgLocalLight1_Position               c42      1
//   mrgLocalLight1_Color                  c43      1
//   mrgLocalLight1_Range                  c44      1
//   mrgLocalLight2_Position               c46      1
//   mrgLocalLight2_Color                  c47      1
//   mrgLocalLight2_Range                  c48      1
//   mrgLocalLight3_Position               c50      1
//   mrgLocalLight3_Color                  c51      1
//   mrgLocalLight3_Range                  c52      1
//   mrgEyeLight_Specular                  c59      1
//   mrgEyeLight_Range                     c60      1
//   mrgEyeLight_Attribute                 c61      1
//   mrgLuminanceRange                     c63      1
//   mrgInShadowScale                      c64      1
//   g_ShadowMapParams                     c65      1
//   g_GI0Scale                            c70      1
//   g_LightScatteringColor                c75      1
//   g_aLightField                         c77      6
//   g_ForceAlphaColor                     c89      1
//   g_MtxView                             c90      3
//   g_SonicSkinFalloffParam               c150     1
//   g_ChaosWaveParamEx                    c151     1
//   sampDif                               s0       1
//   sampSpec                              s1       1
//   sampNrm                               s2       1
//   sampNrm2                              s3       1
//   sampEnv                               s5       1
//   sampBlend                             s6       1
//   g_VerticalShadowMapSampler            s7       1
//   g_FramebufferSampler                  s11      1
//   g_ShadowMapSampler                    s13      1
//

    ps_3_0
    def c0, 2, -1, 1, 0.5
    def c1, 0, 500, 5, 1024
    def c2, -2, 3, 0.5, -0.5
    def c3, 4, 0, 0, 0
    dcl_texcoord v0.xy
    dcl_texcoord2 v1.xyz
    dcl_texcoord3_pp v2
    dcl_texcoord4 v3
    dcl_texcoord6 v4
    dcl_texcoord5_pp v5.xyz
    dcl_texcoord7 v6.xyz
    dcl_texcoord8 v7.xyz
    dcl_color_pp v8.w
    dcl vPos.xy
    dcl_2d s0
    dcl_2d s1
    dcl_2d s2
    dcl_2d s3
    dcl_2d s5
    dcl_2d s6
    dcl_2d s7
    dcl_2d s11
    dcl_2d s13
    add_pp r0.x, -c61.z, c61.y
    rcp_pp r0.x, r0.x
    add r0.yzw, -c22.xxyz, v1.xxyz
    dp3 r1.x, r0.yzww, r0.yzww
    rsq r1.x, r1.x
    mul_pp r0.yzw, r0, r1.x
    rcp r1.x, r1.x
    add r1.x, r1.x, -c60.z
    dp3_pp r1.y, r0.yzww, c23
    add_pp r1.y, r1.y, -c61.z
    mul_sat_pp r0.x, r0.x, r1.y
    mad_pp r1.y, r0.x, c2.x, c2.y
    mul_pp r0.x, r0.x, r0.x
    mul_pp r0.x, r0.x, r1.y
    pow_sat_pp r1.y, r0.x, c61.w
    mov r2.yz, c0
    add r0.x, r2.z, -c61.x
    cmp_pp r0.x, r0.x, c0.z, r1.y
    add r1.y, -c60.z, c60.w
    rcp r1.y, r1.y
    mul_sat r1.x, r1.y, r1.x
    add_pp r1.x, -r1.x, c0.z
    cmp_pp r1.x, -c61.x, r2.z, r1.x
    mul_pp r0.x, r0.x, r1.x
    add r1.x, -c48.z, c48.w
    rcp r1.x, r1.x
    add r1.yzw, -c46.xxyz, v1.xxyz
    dp3 r2.x, r1.yzww, r1.yzww
    rsq r2.x, r2.x
    rcp r2.z, r2.x
    add r2.z, r2.z, -c48.z
    mul_sat r1.x, r1.x, r2.z
    add_pp r1.x, -r1.x, c0.z
    add r2.z, -c40.z, c40.w
    rcp r2.z, r2.z
    add r3.xyz, -c38, v1
    dp3 r2.w, r3, r3
    rsq r2.w, r2.w
    rcp r3.w, r2.w
    add r3.w, r3.w, -c40.z
    mul_sat r2.z, r2.z, r3.w
    add_pp r2.z, -r2.z, c0.z
    add r3.w, -c44.z, c44.w
    rcp r3.w, r3.w
    add r4.xyz, -c42, v1
    dp3 r4.w, r4, r4
    rsq r4.w, r4.w
    rcp r5.x, r4.w
    add r5.x, r5.x, -c44.z
    mul_sat r3.w, r3.w, r5.x
    add_pp r3.w, -r3.w, c0.z
    texld_pp r5, v0, s2
    mul_pp r5.x, r5.w, r5.x
    mad_pp r5.xy, r5, c0.x, c0.y
    dp2add_pp r5.z, r5, -r5, c0.z
    rsq_pp r5.z, r5.z
    rcp_pp r5.z, r5.z
    nrm_pp r6.xyz, v7
    mul_pp r7.xyz, r5.y, r6
    nrm_pp r8.xyz, v6
    mad_pp r5.xyw, r5.x, r8.xyzz, r7.xyzz
    nrm_pp r7.xyz, v2
    mad_pp r5.xyz, r5.z, r7, r5.xyww
    texld_pp r9, v0, s3
    mul_pp r9.x, r9.w, r9.x
    mad_pp r9.xy, r9, c0.x, c0.y
    mul_pp r6.xyz, r6, r9.y
    mad_pp r6.xyz, r9.x, r8, r6
    dp2add_pp r5.w, r9, -r9, c0.z
    rsq_pp r5.w, r5.w
    rcp_pp r5.w, r5.w
    mad_pp r6.xyz, r5.w, r7, r6
    add_pp r5.xyz, r5, r6
    nrm_pp r6.xyz, r5
    add_pp r5.xyz, c22, -v1
    dp3_pp r5.w, r5, r5
    rsq_pp r5.w, r5.w
    mul_pp r7.xyz, r5.w, r5
    mad_pp r4.xyz, r4, -r4.w, r7
    nrm_pp r8.xyz, r4
    dp3_sat_pp r4.x, r8, r6
    mov_pp r4.yz, c1
    mul_pp r4.yz, r4, c20
    max_pp r6.w, r4.y, c0.z
    min r4.y, r6.w, c1.w
    pow r6.w, r4.x, r4.y
    mul_pp r8.xyz, r4.z, c43
    mul_pp r8.xyz, r6.w, r8
    mul_pp r8.xyz, r3.w, r8
    mad_pp r3.xyz, r3, -r2.w, r7
    nrm_pp r9.xyz, r3
    dp3_sat_pp r2.w, r9, r6
    pow r3.x, r2.w, r4.y
    mul_pp r3.yzw, r4.z, c39.xxyz
    mul_pp r3.xyz, r3.x, r3.yzww
    mad_pp r3.xyz, r2.z, r3, r8
    mad_pp r1.yzw, r1, -r2.x, r7.xxyz
    nrm_pp r8.xyz, r1.yzww
    dp3_sat_pp r1.y, r8, r6
    pow r2.x, r1.y, r4.y
    mul_pp r1.yzw, r4.z, c47.xxyz
    mul_pp r1.yzw, r2.x, r1
    mad_pp r1.xyz, r1.x, r1.yzww, r3
    add r1.w, -c52.z, c52.w
    rcp r1.w, r1.w
    add r2.xzw, -c50.xyyz, v1.xyyz
    dp3 r3.x, r2.xzww, r2.xzww
    rsq r3.x, r3.x
    rcp r3.y, r3.x
    mad_pp r2.xzw, r2, -r3.x, r7.xyyz
    nrm_pp r8.xyz, r2.xzww
    dp3_sat_pp r2.x, r8, r6
    pow r3.x, r2.x, r4.y
    add r2.x, r3.y, -c52.z
    mul_sat r1.w, r1.w, r2.x
    add_pp r1.w, -r1.w, c0.z
    mul_pp r2.xzw, r4.z, c51.xyyz
    mul_pp r2.xzw, r3.x, r2
    mad_pp r1.xyz, r1.w, r2.xzww, r1
    max r1.w, v3.z, c1.x
    add r2.x, r1.w, -v3.w
    cmp r3.z, r2.x, c1.x, r1.w
    mov r3.xyw, v3
    texldp_pp r3, r3, s13
    max r1.w, v4.z, c1.x
    add r2.x, r1.w, -v4.w
    cmp r8.z, r2.x, c1.x, r1.w
    mov r8.xyw, v4
    texldp_pp r8, r8, s7
    min_pp r1.w, r8.x, r3.x
    lrp_pp r2.x, v5.z, c0.z, r8.x
    lrp_pp r2.z, v5.z, c0.z, r1.w
    rcp r1.w, v3.w
    mul r3.xy, r1.w, v3
    mad r3.xy, r3, c0.x, c0.y
    abs_sat r3.xy, r3
    add_sat r3.xy, r3, -c65.x
    mul r3.xy, r3, c65.y
    max r1.w, r3.x, r3.y
    lrp_pp r3.x, r1.w, c0.z, r2.z
    lrp_pp r3.y, r1.w, c0.z, r2.x
    mul r1.w, r3.x, c77.w
    mad_pp r2.xzw, r5.xyyz, r5.w, -c10.xyyz
    mad_pp r0.yzw, r5.xxyz, r5.w, -r0
    nrm_pp r5.xyz, r0.yzww
    dp3_sat_pp r0.y, r5, r6
    pow r3.x, r0.y, r4.y
    nrm_pp r5.xyz, r2.xzww
    dp3_sat_pp r0.y, r5, r6
    pow r2.x, r0.y, r4.y
    mul_pp r0.yzw, r4.z, c37.xxyz
    mul_pp r0.yzw, r2.x, r0
    mad_pp r0.yzw, r0, r1.w, r1.xxyz
    mul_pp r1.xyz, c59.w, c59
    mul_pp r1.xyz, r4.z, r1
    mul_pp r1.xyz, r3.x, r1
    mad_pp r0.xyz, r0.x, r1, r0.yzww
    mul_pp r0.xyz, r0, c18
    mul r0.w, c64.z, c64.x
    mov r1.xz, c64
    mad r1.x, r1.x, -r1.z, r1.z
    mad_pp r0.w, r1.w, r1.x, r0.w
    mad r1.x, r1.w, c0.w, c0.w
    mul r4, r6.y, c91
    mad r4, r6.x, c90, r4
    mad r4, r6.z, c92, r4
    dp4 r1.y, r4, r4
    rsq r1.y, r1.y
    mul r1.yz, r1.y, r4.xxyw
    mad_pp r1.yz, r1, c2.xzww, c2.z
    texld_pp r5, r1.yzzw, s5
    mul r1.yzw, r5.xxyz, c63.x
    mad_pp r1.yzw, r1, r5.w, r5.xxyz
    texld_pp r5, v0, s1
    mul_pp r1.yzw, r1, r5.w
    mul_pp r1.yzw, r0.w, r1
    mul_pp r1.yzw, r1, c17.xxyz
    mad_pp r0.xyz, r0, r5, r1.yzww
    mul_pp r1.yzw, r6.xxyz, r6.xxyz
    mad_pp r2.xzw, r6.xyyz, c0.w, c0.w
    mov r5.xyz, c79
    add r3.xzw, -r5.xyyz, c80.xyyz
    mad r5.xyz, r2.z, r3.xzww, c79
    mul r5.xyz, r1.z, r5
    mov r8.xyz, c77
    add r8.xyz, -r8, c78
    mad r9.xyz, r2.x, r8, c77
    mad_pp r5.xyz, r1.y, r9, r5
    mov r9.xyz, c81
    add r9.xyz, -r9, c82
    mad r2.xzw, r2.w, r9.xyyz, c81.xyyz
    mad_pp r1.yzw, r1.w, r2.xxzw, r5.xxyz
    mul_pp r2.xzw, r1.yyzw, c17.xyyz
    mul r2.xzw, r2, c70.w
    mad r1.yzw, r1, c17.xxyz, -r2.xxzw
    mad_pp r1.yzw, r3.y, r1, r2.xxzw
    add_pp r1.yzw, r1, c0.z
    mul_pp r1.yzw, r1, c16.xxyz
    mul r0.w, c24.w, vPos.y
    mad_pp r5.y, r0.w, -c0.x, -c0.y
    dp2add_pp r5.x, vPos.x, c24.z, r2.y
    mad_pp r2.xy, r4, c151.y, r5
    mad r2.xy, r2, c2.zwzw, c2.z
    texld_pp r2, r2, s11
    texld_pp r4, v0, s0
    mad_pp r1.yzw, r1, r4.xxyz, -r2.xxyz
    mul_pp r0.w, r4.w, c16.w
    mul_pp r0.w, r0.w, v8.w
    mul_pp r4.w, r0.w, c21.x
    texld_pp r5, v0, s6
    mad_pp r1.yzw, r5.x, r1, r2.xxyz
    mad_pp r0.xyz, r0, v2.w, r1.yzww
    dp3_pp r0.w, r7, r6
    add_sat_pp r1.y, -r0.w, c0.z
    add_pp r0.w, r0.w, r0.w
    mad_pp r2.xyz, r0.w, r6, -r7
    pow_pp r0.w, r1.y, c150.z
    mul r0.w, r0.w, c150.y
    mad_pp r0.w, r0.w, r1.x, c150.x
    mad_pp r1.xyz, r2, c0.w, c0.w
    mul_pp r2.xyz, r2, r2
    mad r3.xyz, r1.y, r3.xzww, c79
    mul r3.xyz, r2.y, r3
    mad r1.xyw, r1.x, r8.xyzz, c77.xyzz
    mad r5.xyz, r1.z, r9, c81
    mad_pp r1.xyz, r2.x, r1.xyww, r3
    mad_pp r1.xyz, r2.z, r5, r1
    mad_pp r4.xyz, r1, r0.w, r0
    mul_pp r0, r4, c89
    min_pp r1, r0, c3.x
    mul_pp r0.xyz, c75, v5.y
    mad_pp oC0.xyz, r1, v5.x, r0
    mov_pp oC0.w, r1.w

// approximately 272 instruction slots used (9 texture, 263 arithmetic)