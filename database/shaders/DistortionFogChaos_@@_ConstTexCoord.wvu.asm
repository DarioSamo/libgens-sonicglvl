//
// Generated by Microsoft (R) HLSL Shader Compiler 9.29.952.3111
//
// Parameters:
//
//   float4 g_ChaosWaveParamEx;
//   float4 g_ChaosWaveParamY;
//   float4 g_ChaosWaveParamZX;
//   float4 g_EyePosition;
//   float4 g_LightScatteringFarNearScale;
//   float4 g_LightScattering_ConstG_FogDensity;
//   float4 g_LightScattering_Ray_Mie_Ray2_Mie2;
//   row_major float4x4 g_MtxProjection;
//   row_major float4x4 g_MtxView;
//   row_major float4x4 g_MtxWorld;
//   float4 g_TimeParam;
//   float4 g_VerticalLightDirection;
//   float4 mrgDistortionParam;
//   float4 mrgGIAtlasParam;
//   float4 mrgGlobalLight_Direction;
//   float4 mrgTexcoordIndex[4];
//   float4 mrgTexcoordOffset[2];
//
//
// Registers:
//
//   Name                                Reg   Size
//   ----------------------------------- ----- ----
//   g_MtxProjection                     c0       4
//   g_MtxView                           c4       4
//   g_MtxWorld                          c8       4
//   g_EyePosition                       c178     1
//   mrgGlobalLight_Direction            c183     1
//   mrgGIAtlasParam                     c186     1
//   mrgTexcoordIndex                    c187     1
//   mrgTexcoordOffset                   c191     2
//   g_LightScattering_Ray_Mie_Ray2_Mie2 c197     1
//   g_LightScattering_ConstG_FogDensity c198     1
//   g_LightScatteringFarNearScale       c199     1
//   g_VerticalLightDirection            c211     1
//   g_TimeParam                         c212     1
//   mrgDistortionParam                  c246     1
//   g_ChaosWaveParamY                   c247     1
//   g_ChaosWaveParamZX                  c248     1
//   g_ChaosWaveParamEx                  c249     1
//

    vs_3_0
    def c225, 0.0208350997, -0.0851330012, 0.180141002, -0.330299497
    def c226, 0.999866009, -2, 1.57079637, -3.14159274
    def c227, 0.159154937, 0.5, 6.28318548, -3.14159274
    def c228, 1.44269502, 1.5, 0, 0
    def c221, 1, 0, 0, 0
    def c222, 0, 1, 0, 0
    def c223, 0, 0, 1, 0
    def c224, 0, 0, 0, 1
    dcl_position v0
    dcl_texcoord v1
    dcl_texcoord1 v2
    dcl_texcoord2 v3
    dcl_texcoord3 v4
    dcl_normal v5
    dcl_tangent v6
    dcl_binormal v7
    dcl_color v8
    dcl_position o0
    dcl_texcoord o1
    dcl_texcoord2 o2
    dcl_texcoord3 o3.xyz
    dcl_texcoord4 o4
    dcl_texcoord8 o5.xyz
    dcl_color o6
    dcl_texcoord6 o7.xy
    max r0.x, v0_abs.x, v0_abs.z
    rcp r0.x, r0.x
    min r0.y, v0_abs.x, v0_abs.z
    mul r0.x, r0.x, r0.y
    mul r0.y, r0.x, r0.x
    mad r0.z, r0.y, c225.x, c225.y
    mad r0.z, r0.y, r0.z, c225.z
    mad r0.z, r0.y, r0.z, c225.w
    mad r0.y, r0.y, r0.z, c226.x
    mul r0.x, r0.y, r0.x
    mad r0.y, r0.x, c226.y, c226.z
    slt r0.z, v0_abs.x, v0_abs.z
    mad r0.x, r0.y, r0.z, r0.x
    slt r0.y, v0.x, -v0.x
    mad r0.x, r0.y, c226.w, r0.x
    add r0.y, r0.x, r0.x
    min r0.z, v0.x, v0.z
    slt r0.z, r0.z, -r0.z
    max r0.w, v0.x, v0.z
    sge r0.w, r0.w, -r0.w
    mul r0.z, r0.w, r0.z
    mad r0.x, r0.z, -r0.y, r0.x
    mad r0.x, v0.y, c249.x, r0.x
    rcp r0.y, c248.x
    mul r0.x, r0.y, r0.x
    mov r1.x, c248.x
    mad r0.x, c212.x, r1.x, -r0.x
    mad r0.x, r0.x, c227.x, c227.y
    frc r0.x, r0.x
    mad r0.x, r0.x, c227.z, c227.w
    sincos r1.y, r0.x
    slt r0.x, -r1.y, r1.y
    slt r0.y, r1.y, -r1.y
    pow r0.z, r1_abs.y, c248.x
    add r0.x, -r0.y, r0.x
    mul r0.x, r0.z, r0.x
    mul r0.x, r0.x, c248.x
    rcp r0.y, c247.x
    mul r0.y, r0.y, v0.y
    mov r1.x, c247.x
    mad r0.y, c212.x, r1.x, -r0.y
    mad r0.y, r0.y, c227.x, c227.y
    frc r0.y, r0.y
    mad r0.y, r0.y, c227.z, c227.w
    sincos r1.y, r0.y
    slt r0.y, -r1.y, r1.y
    slt r0.z, r1.y, -r1.y
    pow r0.w, r1_abs.y, c247.x
    add r0.y, -r0.z, r0.y
    mul r0.y, r0.w, r0.y
    mad r0.x, r0.y, c247.x, r0.x
    mul r0.xyz, r0.x, v5
    mov r1.xyz, v0
    mad r0.xyz, r0, v8.w, r1
    mul r1, r0.y, c9
    mad r1, r0.x, c8, r1
    mad r0, r0.z, c10, r1
    mad r0, v0.w, c11, r0
    mul r1, r0.y, c5
    mad r1, r0.x, c4, r1
    mad r1, r0.z, c6, r1
    mad r1, r0.w, c7, r1
    add r0.xyz, -r0, c178
    nrm r2.xyz, r0
    dp3 r0.x, -c183, r2
    mul r2, r1.y, c1
    mad r2, r1.x, c0, r2
    mad r2, r1.z, c2, r2
    mad o0, r1.w, c3, r2
    add r0.yz, c191.xxyw, v1.xxyw
    slt r0.w, c187.x, -c187.x
    frc r2.x, c187.x
    add r2.y, -r2.x, c187.x
    slt r2.x, -r2.x, r2.x
    mad r0.w, r0.w, r2.x, r2.y
    mova a0.x, r0.w
    mul r0.yz, r0, c221[a0.x].x
    add r2.xy, c191.zwzw, v2
    mad r0.yz, c221[a0.x].y, r2.xxyw, r0
    mad o1.zw, r2.xyxy, c186.xyxy, c186
    add r2.xy, c192, v3
    mad r0.yz, c221[a0.x].z, r2.xxyw, r0
    add r2.xy, c192.zwzw, v4
    mad o1.xy, c221[a0.x].w, r2, r0.yzzw
    mov o5.xyz, -v7
    mov r2.xyw, -c221.w
    mov r2.z, -c246.x
    add r2, r1, r2
    mul r0.yz, r2.y, c1.xzww
    mad r0.yz, r2.x, c0.xzww, r0
    mad r0.yz, r2.z, c2.xzww, r0
    mad o7.xy, r2.w, c3.zwzw, r0.yzzw
    mad r0.y, c198.z, r0.x, c198.y
    mad r0.x, r0.x, r0.x, c221.x
    pow r1.x, r0_abs.y, c228.y
    rcp r0.y, r1.x
    mul r0.y, r0.y, c198.x
    mul r0.y, r0.y, c197.w
    mad r0.x, c197.z, r0.x, r0.y
    add r0.y, c197.y, c197.x
    rcp r0.z, r0.y
    mul r0.x, r0.z, r0.x
    add r0.z, -r1.z, -c199.y
    mov o2.w, -r1.z
    mul_sat r0.z, r0.z, c199.x
    mul r0.z, r0.z, c199.z
    mul r0.y, r0.z, -r0.y
    mul r0.y, r0.y, c228.x
    exp r0.y, r0.y
    add r0.z, -r0.y, c221.x
    mov o4.x, r0.y
    mul r0.x, r0.z, r0.x
    mul o4.y, r0.x, c199.w
    mad r0, v5.xyzx, c221.xxxw, c221.yyzx
    dp4_sat r0.x, r0, c211
    add o4.z, -r0.x, c221.x
    mov o2.xyz, v5
    mov o3.xyz, v6
    mov o4.w, c221.w
    mad o6, v8.xyzx, c221.xxxw, c221.yyzx

// approximately 142 instruction slots used
