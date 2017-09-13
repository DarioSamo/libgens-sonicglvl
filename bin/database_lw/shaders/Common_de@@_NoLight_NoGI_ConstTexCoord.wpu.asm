//
// Generated by Microsoft (R) HLSL Shader Compiler 9.29.952.3111
//
// Parameters:
//
//   float4 g_Ambient;
//   float4 g_Diffuse;
//   float4 g_EyePosition;
//   float4 g_aLightField[6];
//   float4 mrgGlobalLight_Diffuse;
//   float4 mrgGlobalLight_Direction;
//   float4 mrgInShadowScale;
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
//   g_EyePosition            c22      1
//   mrgGlobalLight_Diffuse   c36      1
//   mrgLuminanceRange        c63      1
//   mrgInShadowScale         c64      1
//   g_aLightField            c77      6
//   sampDif                  s0       1
//   sampEnv                  s5       1
//   sampDLScatter            s9       1
//

    ps_3_0
    def c0, 0.5, -1, 1, 0
    def c1, 0.25, 0, 0, 0
    dcl_texcoord v0.xy
    dcl_texcoord2 v1.xyz
    dcl_texcoord3_pp v2
    dcl_texcoord5 v3.xy
    dcl_color_pp v4
    dcl_2d s0
    dcl_2d s5
    dcl_2d s9
    texld_pp r0, v0, s0
    mul_pp r0.w, r0.w, c16.w
    mul_pp oC0.w, r0.w, v4.w
    mul r0.w, c64.z, c64.x
    mov r1.xz, c64
    mad r1.x, r1.x, -r1.z, r1.z
    mad_pp r0.w, c77.w, r1.x, r0.w
    mul r0.w, r0.w, c63.x
    add_pp r1, c22.xyzx, -v1.xyzx
    dp3_pp r2.x, r1.yzww, r1.yzww
    rsq_pp r2.x, r2.x
    mul r1, r1, r2.x
    nrm r2, v2.xyzx
    dp3 r3.x, r1.yzww, r2.yzww
    add r3.x, r3.x, r3.x
    mad r1, r3.x, r2, -r1
    mul r3, r1.wyzw, c0.yyzw
    mad r4, r1, c0.zyyw, c0.wwwx
    cmp r1, r1.z, r3, r4
    add r1.z, r1.z, c0.z
    rcp r1.z, r1.z
    mad r1.xy, r1.yxzw, r1.z, c0.z
    mad_pp r3.x, r1.y, c1.x, r1.w
    mul_pp r3.y, r1.x, c0.x
    texld_pp r1, r3, s5
    mul_pp r1.xyz, r0.w, r1
    mul_pp r1.xyz, r1, v2.w
    mad_pp r3.xyz, r2.wyzw, c0.x, c0.x
    mov r4.xyz, c80
    mad r4.xyz, r4, r3.y, c79
    mul_pp r5.xyz, r2.wyzw, r2.wyzw
    dp3_sat_pp r0.w, r2.wyzw, -c10
    mul r0.w, r0.w, c77.w
    mul_pp r2.xyz, r0.w, c36
    mul r4.xyz, r4, r5.y
    mov r6.xyz, c77
    mad r3.xyw, c78.xyzz, r3.x, r6.xyzz
    mov r6.xyz, c82
    mad r6.xyz, r6, r3.z, c81
    mad_pp r3.xyz, r5.x, r3.xyww, r4
    mad_pp r3.xyz, r5.z, r6, r3
    mad_pp r2.xyz, r3, c17, r2
    mul_pp r2.xyz, r2, c16
    mad_pp r0.xyz, r2, r0, r1
    mul_pp r0.xyz, r0, v4
    texld_pp r1, v3.yxzw, s9
    mad_pp oC0.xyz, r0, r1.w, r1

// approximately 49 instruction slots used (3 texture, 46 arithmetic)