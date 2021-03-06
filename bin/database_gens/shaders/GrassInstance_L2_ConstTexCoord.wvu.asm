//
// Generated by Microsoft (R) HLSL Shader Compiler 9.29.952.3111
//
// Parameters:
//
//   float4 g_EyeDirection;
//   float4 g_InstanceTypes[32];
//   float4 g_LightScatteringFarNearScale;
//   float4 g_LightScattering_ConstG_FogDensity;
//   float4 g_LightScattering_Ray_Mie_Ray2_Mie2;
//   row_major float4x4 g_MtxProjection;
//   row_major float4x4 g_MtxView;
//   float4 mrgGlobalLight_Direction;
//
//
// Registers:
//
//   Name                                Reg   Size
//   ----------------------------------- ----- ----
//   g_MtxProjection                     c0       4
//   g_MtxView                           c4       4
//   g_InstanceTypes                     c28     32
//   g_EyeDirection                      c179     1
//   mrgGlobalLight_Direction            c183     1
//   g_LightScattering_Ray_Mie_Ray2_Mie2 c197     1
//   g_LightScattering_ConstG_FogDensity c198     1
//   g_LightScatteringFarNearScale       c199     1
//

    vs_3_0
    def c8, 1.44269502, 1, 1.5, 0
    def c9, 0.5, 6.28318548, -3.14159274, 0.00392156886
    def c10, 0.300000012, 0.699999988, 0, 0
    dcl_position v0
    dcl_texcoord v1
    dcl_texcoord1 v2
    dcl_texcoord2 v3
    dcl_texcoord3 v4
    dcl_color1 v5
    dcl_position o0
    dcl_texcoord o1
    dcl_texcoord1 o2
    dcl_color o3
    mad r0.xy, v4, c9.x, c9.x
    frc r0.xy, r0
    mad r0.xy, r0, c9.y, c9.z
    sincos r1.xy, r0.x
    sincos r2.xy, r0.y
    add r0.x, c9.x, v3.x
    frc r0.y, r0.x
    add r0.x, -r0.y, r0.x
    add r0.x, r0.x, r0.x
    mova a0.x, r0.x
    mul r0.xyz, v0, c28[a0.x]
    mul r3.xyz, r1.xyxw, r0.yyzw
    mad r0.y, r0.z, -r1.y, r3.x
    mad r0.z, r1.x, c10.x, c10.y
    mul o3.w, r0.z, v5.w
    add r0.z, r3.z, r3.y
    mul r0.zw, r2.xyyx, r0.z
    mad r1.xy, v3.zwzw, c9.w, c9.x
    frc r1.xy, r1
    mad r1.xy, r1, c9.y, c9.z
    sincos r3.xy, r1.x
    sincos r4.xy, r1.y
    mul r1.xy, r0.y, r3
    mad r0.y, r0.x, -r2.y, r0.w
    mad r0.x, r0.x, r2.x, r0.z
    mad r0.z, r0.y, r3.x, r1.y
    mad r1.y, r0.y, -r3.y, r1.x
    mul r0.yz, r4.xyxw, r0.z
    mad r1.x, r0.x, r4.x, r0.y
    mad r1.z, r0.x, -r4.y, r0.z
    add r0.xyz, r1, v2
    mul r1, r0.y, c5
    mad r1, r0.x, c4, r1
    mad r0, r0.z, c6, r1
    add r0, r0, c7
    mul r1, r0.y, c1
    mad r1, r0.x, c0, r1
    mad r1, r0.z, c2, r1
    mad o0, r0.w, c3, r1
    add r0.x, -r0.z, -c199.y
    mul_sat r0.x, r0.x, c199.x
    mul r0.x, r0.x, c199.z
    add r0.yz, -c29[a0.x].xxyw, c29[a0.x].xzww
    mad o1.xy, v1, r0.yzzw, c29[a0.x]
    add r0.y, c197.y, c197.x
    mul r0.x, r0.x, -r0.y
    rcp r0.y, r0.y
    mul r0.x, r0.x, c8.x
    exp r0.x, r0.x
    add r0.z, -r0.x, c8.y
    mov o2.x, r0.x
    mov r1.xyz, c183
    dp3 r0.x, -r1, c179
    mad r0.w, c198.z, r0.x, c198.y
    mad r0.x, r0.x, r0.x, c8.y
    pow r1.x, r0_abs.w, c8.z
    rcp r0.w, r1.x
    mul r0.w, r0.w, c198.x
    mul r0.w, r0.w, c197.w
    mad r0.x, c197.z, r0.x, r0.w
    mul r0.x, r0.y, r0.x
    mul r0.x, r0.z, r0.x
    mul o2.y, r0.x, c199.w
    mov o1.zw, c8.w
    mov o2.zw, c8.w
    mov o3.xyz, v5

// approximately 96 instruction slots used
