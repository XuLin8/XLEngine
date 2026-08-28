#type vertex
#version 450 core

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec2 a_TexCoord;
layout(location = 4) in vec4 a_Color;
layout(location = 5) in int a_EntityID;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

out vec3 v_WorldPos;
out vec3 v_Normal;
out vec4 v_Color;
out flat int v_EntityID;

void main()
{
	vec4 worldPos = u_Model * vec4(a_Pos, 1.0);
	v_WorldPos = worldPos.xyz;
	v_Normal = normalize(mat3(u_Model) * a_Normal);
	v_Color = a_Color;
	v_EntityID = a_EntityID;
	gl_Position = u_ViewProjection * worldPos;
}

#type fragment
#version 450 core

in vec3 v_WorldPos;
in vec3 v_Normal;
in vec4 v_Color;
in flat int v_EntityID;

layout(location = 0) out vec4 color;
layout(location = 1) out int color2;
layout(location = 2) out vec4 color3; // world-space normal (encoded) for ink outline edge detection

uniform vec3 u_CameraPos;
uniform vec4 u_MaterialColor;

// Directional light (sun)
uniform vec3 u_SunDirection;
uniform vec3 u_SunColor;

// Ambient
uniform vec3 u_AmbientColor;

// Point lights (toxic green / ember)
uniform vec3 u_PL0Pos;
uniform vec3 u_PL0Color;
uniform float u_PL0Radius;
uniform vec3 u_PL1Pos;
uniform vec3 u_PL1Color;
uniform float u_PL1Radius;

// Fog
uniform vec3 u_FogColor;
uniform float u_FogDensity;

// 3-step band quantization for toon shading (keeps the dark side above pure black)
float toonBands(float x)
{
	float b = floor(x * 3.0) / 3.0;
	return b * 0.5 + 0.5;
}

void main()
{
	vec3 N = normalize(v_Normal);
	vec3 base = u_MaterialColor.rgb * v_Color.rgb;

	vec3 V = normalize(u_CameraPos - v_WorldPos);

	// Sun (directional) toon lighting
	vec3 L = normalize(u_SunDirection);
	float ndl = max(dot(N, L), 0.0);
	vec3 col = u_AmbientColor + u_SunColor * toonBands(ndl);

	// Point light 0 (toxic green), smooth falloff
	vec3 toL0 = u_PL0Pos - v_WorldPos;
	float d0 = length(toL0);
	float att0 = clamp(1.0 - d0 / u_PL0Radius, 0.0, 1.0);
	att0 *= att0;
	float ndlp0 = max(dot(N, normalize(toL0)), 0.0);
	col += u_PL0Color * ndlp0 * att0 * 1.6;

	// Point light 1 (ember)
	vec3 toL1 = u_PL1Pos - v_WorldPos;
	float d1 = length(toL1);
	float att1 = clamp(1.0 - d1 / u_PL1Radius, 0.0, 1.0);
	att1 *= att1;
	float ndlp1 = max(dot(N, normalize(toL1)), 0.0);
	col += u_PL1Color * ndlp1 * att1 * 1.6;

	vec3 outCol = base * col;

	// Rim light: subtle edge highlight for hand-drawn silhouette pop
	vec3 rimColor = vec3(0.79, 0.78, 0.72);
	float rim = pow(1.0 - max(dot(N, V), 0.0), 3.0);
	outCol += rimColor * rim * 0.2;

	// exp2 exponential fog
	float dist = length(v_WorldPos - u_CameraPos);
	float fogF = 1.0 - exp(-u_FogDensity * u_FogDensity * dist * dist);
	outCol = mix(outCol, u_FogColor, clamp(fogF, 0.0, 1.0));

	color = vec4(outCol, 1.0);
	color2 = v_EntityID;
	color3 = vec4(N * 0.5 + 0.5, 1.0); // encode [-1,1] -> [0,1] for RGBA16F attachment
}
