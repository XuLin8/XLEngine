#type vertex
#version 450 core

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = vec4(a_Pos, 1.0);
}

#type fragment
#version 450 core

in vec2 v_TexCoord;

layout(location = 0) out vec4 color;

uniform sampler2D u_SceneColor;
uniform sampler2D u_SceneDepth;
uniform sampler2D u_SceneNormal;

// Camera clip planes for linearizing the depth buffer in the diagnostic view.
// Raw depth is non-linear (z/w) and with a 0.1..1000 clip range the whole
// terrain sits at ~0.999, so showing it raw reads as pure white even though
// the buffer is perfectly valid.
uniform float u_Near;
uniform float u_Far;

// --- 2x2 diagnostic split view ---
// bottom-left  : raw scene color (as produced by Toon.glsl)
// bottom-right : ink edge map (NMS output, gray = 0..1)
// top-left     : world normals (decoded, colored)
// top-right    : linearized depth (gray 0..1, near=black far=white, sky=dark backdrop)

// Undo the perspective divide: z_buf -> view-space distance (positive).
float linearDepth(float zBuf)
{
	return (2.0 * u_Near * u_Far) / (u_Far + u_Near - (2.0 * zBuf - 1.0) * (u_Far - u_Near));
}

float edgeAt(vec2 uv)
{
	vec2 t = 1.0 / vec2(textureSize(u_SceneColor, 0));
	float zd = texture(u_SceneDepth, uv).r;
	vec3 nC = texture(u_SceneNormal, uv).xyz * 2.0 - 1.0;

	float nE = 0.0;
	for (int r = 1; r <= 3; ++r)
	{
		vec2 o = t * float(r);
		nE = max(nE, 1.0 - dot(nC, texture(u_SceneNormal, uv + vec2( o.x, 0.0)).xyz * 2.0 - 1.0));
		nE = max(nE, 1.0 - dot(nC, texture(u_SceneNormal, uv + vec2(-o.x, 0.0)).xyz * 2.0 - 1.0));
		nE = max(nE, 1.0 - dot(nC, texture(u_SceneNormal, uv + vec2(0.0,  o.y)).xyz * 2.0 - 1.0));
		nE = max(nE, 1.0 - dot(nC, texture(u_SceneNormal, uv + vec2(0.0, -o.y)).xyz * 2.0 - 1.0));
	}

	float dE = 0.0;
	dE += abs(texture(u_SceneDepth, uv + vec2( t.x, 0.0)).r - zd);
	dE += abs(texture(u_SceneDepth, uv + vec2(-t.x, 0.0)).r - zd);
	dE += abs(texture(u_SceneDepth, uv + vec2(0.0,  t.y)).r - zd);
	dE += abs(texture(u_SceneDepth, uv + vec2(0.0, -t.y)).r - zd);

	float nAmp = pow(min(nE, 1.0), 0.5);
	return smoothstep(0.10, 0.28, nAmp) * 1.8 + smoothstep(0.0015, 0.04, dE) * 0.7;
}

vec2 edgeDir(vec2 uv)
{
	vec2 t = 1.0 / vec2(textureSize(u_SceneColor, 0));
	vec3 nC = texture(u_SceneNormal, uv).xyz * 2.0 - 1.0;
	float ex = 1.0 - dot(nC, texture(u_SceneNormal, uv + vec2( t.x, 0.0)).xyz * 2.0 - 1.0);
	float wx = 1.0 - dot(nC, texture(u_SceneNormal, uv + vec2(-t.x, 0.0)).xyz * 2.0 - 1.0);
	float ny = 1.0 - dot(nC, texture(u_SceneNormal, uv + vec2(0.0,  t.y)).xyz * 2.0 - 1.0);
	float sy = 1.0 - dot(nC, texture(u_SceneNormal, uv + vec2(0.0, -t.y)).xyz * 2.0 - 1.0);
	if (ex >= wx && ex >= ny && ex >= sy) return vec2( 1.0,  0.0);
	if (wx >  ex && wx >= ny && wx >= sy) return vec2(-1.0,  0.0);
	if (ny >  ex && ny >  wx && ny >= sy) return vec2( 0.0,  1.0);
	return vec2(0.0, -1.0);
}

void main()
{
	vec2 texel = 1.0 / vec2(textureSize(u_SceneColor, 0));
	vec3 scene = texture(u_SceneColor, v_TexCoord).rgb;
	vec3 nrm = texture(u_SceneNormal, v_TexCoord).xyz * 2.0 - 1.0;
	float depth = texture(u_SceneDepth, v_TexCoord).r;

	float e0 = edgeAt(v_TexCoord);
	vec2 dn = edgeDir(v_TexCoord);
	float eF = edgeAt(v_TexCoord + dn * texel);
	float eB = edgeAt(v_TexCoord - dn * texel);
	float edge = (e0 >= eF && e0 >= eB) ? e0 : 0.0;

	vec3 outCol;
	if (v_TexCoord.x < 0.5 && v_TexCoord.y < 0.5)      outCol = scene;                    // raw scene
	else if (v_TexCoord.x >= 0.5 && v_TexCoord.y < 0.5) outCol = vec3(edge);              // edge map
	else if (v_TexCoord.x < 0.5 && v_TexCoord.y >= 0.5) outCol = nrm * 0.5 + 0.5;         // normals
	else
	{
		// Normalize against a short max distance (400) instead of u_Far (1000)
		// so the terrain spans a wider gray band instead of being crushed dark.
		// Sky/background (cleared depth == 1.0) is painted as a dark backdrop
		// so the terrain depth bands stand out.
		float ld = linearDepth(depth);
		float v = (depth > 0.9995) ? 0.05 : clamp(ld / 400.0, 0.0, 1.0);
		outCol = vec3(v);
	}

	color = vec4(outCol, 1.0);
}
