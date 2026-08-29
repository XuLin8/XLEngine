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

// Camera clip planes for linearizing the depth buffer (diagnostic + fog)
uniform float u_Near;
uniform float u_Far;

// Viewport mode: 0 = full-screen stylized output, 1 = 2x2 diagnostic split
uniform int u_ShowDiagnostics;

// Day-night cycle clock in [0,1) — drives sky gradient / sun-moon / dust
uniform float u_Time;

// For parallax cloud bands: camera position + inverse view-projection
uniform vec3 u_CameraPos;
uniform mat4 u_InvViewProj;

// Stylized grade parameters (art-direction locked)
uniform float u_OutlineStrength;
uniform float u_GrainAmount;
uniform float u_Contrast;
uniform float u_Saturation;
uniform float u_Brightness;
uniform float u_Vignette;
uniform float u_FogStrength;

// ---------------------------------------------------------------------------
// Utility
// ---------------------------------------------------------------------------

// Undo the perspective divide: z_buf -> view-space distance (positive).
float linearDepth(float zBuf)
{
	return (2.0 * u_Near * u_Far) / (u_Far + u_Near - (2.0 * zBuf - 1.0) * (u_Far - u_Near));
}

float hash12(vec2 p)
{
	vec3 p3 = fract(vec3(p.xyx) * 0.1031);
	p3 += dot(p3, p3.yzx + 33.33);
	return fract((p3.x + p3.y) * p3.z);
}

float vnoise(vec2 p)
{
	vec2 i = floor(p);
	vec2 f = fract(p);
	vec2 u = f * f * (3.0 - 2.0 * f);
	float a = hash12(i);
	float b = hash12(i + vec2(1.0, 0.0));
	float c = hash12(i + vec2(0.0, 1.0));
	float d = hash12(i + vec2(1.0, 1.0));
	return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

float fbm2(vec2 p)
{
	float s = 0.0;
	float a = 0.5;
	for (int i = 0; i < 5; i++)
	{
		s += a * vnoise(p);
		p *= 2.03;
		a *= 0.5;
	}
	return s;
}

// Ink edge detection (NMS output, gray = 0..1) from the normal + depth buffers.
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

// Reconstruct the world-space view ray direction for this fragment.
vec3 worldDir(vec2 uv)
{
	vec4 d = u_InvViewProj * vec4(uv * 2.0 - 1.0, 1.0, 1.0);
	return normalize(d.xyz / d.w - u_CameraPos);
}

// ---------------------------------------------------------------------------
// Day-night derived factors
// ---------------------------------------------------------------------------

float dayFactor()      { return clamp(1.0 - abs(u_Time - 0.5) * 2.0, 0.0, 1.0); }  // 1 at noon, 0 at midnight
float warmFactor()
{
	return clamp(max(1.0 - abs(u_Time - 0.25) / 0.12, 1.0 - abs(u_Time - 0.75) / 0.12), 0.0, 1.0); // dawn/dusk
}

// ---------------------------------------------------------------------------
// Sky: gradient + stars + sun/moon, ink-dark clouds as parallax bands
// ---------------------------------------------------------------------------

vec3 skyColor(vec3 rd, float day, float warm)
{
	float h = clamp(rd.y * 2.0 + 0.2, 0.0, 1.0); // 0 near horizon, 1 at zenith

	vec3 nightTop = vec3(0.030, 0.040, 0.055);
	vec3 nightHor = vec3(0.090, 0.100, 0.075);
	vec3 dayTop   = vec3(0.42, 0.50, 0.58);
	vec3 dayHor   = vec3(0.66, 0.64, 0.55);
	vec3 warmHor  = vec3(0.90, 0.62, 0.38);

	vec3 top = mix(nightTop, dayTop, day);
	vec3 hor = mix(nightHor, dayHor, day);
	hor = mix(hor, warmHor, warm * 0.8);

	vec3 col = mix(hor, top, pow(h, 0.6));

	// Sparse twinkling star field on the upper hemisphere at night
	if (rd.y > 0.0 && day < 0.35)
	{
		vec2 cell = floor(rd.xy / 0.02);
		float tw = hash12(cell);
		float star = step(0.985, tw) * (1.0 - day / 0.35);
		star *= 0.55 + 0.45 * sin(u_Time * 50.0 + hash12(cell + 13.0) * 40.0);
		col += vec3(0.85, 0.90, 1.0) * star * 1.3;
	}

	// Sun (moon at night) disc + soft halo, matching the Renderer3D sun arc
	vec3 sunDir = normalize(vec3(-40.0, 24.0 + 60.0 * day, -30.0));
	float sd = dot(rd, sunDir);
	vec3 sunCol = mix(vec3(0.72, 0.80, 1.0), vec3(0.98, 0.86, 0.62), day);
	sunCol = mix(sunCol, vec3(1.0, 0.70, 0.40), warm * 0.9);
	col += sunCol * smoothstep(0.9992, 0.9999, sd) * 2.2;
	col += sunCol * pow(max(sd, 0.0), 32.0) * 0.12;

	return col;
}

// Cloud bands as dark ink silhouettes: two parallax layers at different
// world heights, elongated along the band axis and drifting with time.
float cloudDensity(vec3 ro, vec3 rd, float time)
{
	if (rd.y <= 0.02)
		return 0.0;

	float c = 0.0;

	// Far high layer (slow drift, thin)
	float t1 = (58.0 - ro.y) / rd.y;
	if (t1 > 0.0)
	{
		vec3 p = ro + rd * t1;
		vec2 q = vec2(p.x * 0.008, p.z * 0.05);
		c += smoothstep(0.42, 0.62, fbm2(q)) * 0.55;
	}

	// Near low layer (faster drift, denser)
	float t2 = (40.0 - ro.y) / rd.y;
	if (t2 > 0.0)
	{
		vec3 p = ro + rd * t2 + vec3(time * 2.5, 0.0, 0.0);
		vec2 q = vec2(p.x * 0.014, p.z * 0.07);
		c += smoothstep(0.40, 0.60, fbm2(q)) * 0.45;
	}

	return clamp(c, 0.0, 1.0);
}

// Sparse drifting light dust motes (screen-space, subtle).
vec3 dustMotes(vec2 uv, float time)
{
	vec3 acc = vec3(0.0);
	for (int L = 0; L < 2; L++)
	{
		float scale = 22.0 + float(L) * 16.0;
		vec2 p = uv * scale + vec2(time * (0.35 + float(L) * 0.22), 0.0);
		vec2 cell = floor(p);
		vec2 f = fract(p) - 0.5;
		float h = hash12(cell + float(L) * 91.0);
		if (h > 0.74)
		{
			float mote = smoothstep(0.07, 0.0, length(f));
			float bright = 0.25 + 0.75 * hash12(cell + float(L) * 37.0);
			bright *= 0.55 + 0.45 * sin(time * 2.0 + h * 40.0);
			vec3 tint = (h > 0.93) ? vec3(0.62, 0.72, 0.29) : vec3(0.85, 0.86, 0.80);
			acc += tint * mote * bright * (0.5 / float(L + 1));
		}
	}
	return acc;
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

void main()
{
	vec2 texel = 1.0 / vec2(textureSize(u_SceneColor, 0));
	vec3 scene = texture(u_SceneColor, v_TexCoord).rgb;
	vec3 nrm = texture(u_SceneNormal, v_TexCoord).xyz * 2.0 - 1.0;
	float depth = texture(u_SceneDepth, v_TexCoord).r;

	// ---- Diagnostic 2x2 split view (opt-in) ----
	// bottom-left  : raw scene color   bottom-right : ink edge map
	// top-left     : world normals     top-right    : linearized depth
	if (u_ShowDiagnostics == 1)
	{
		vec3 outCol;
		if (v_TexCoord.x < 0.5 && v_TexCoord.y < 0.5)      outCol = scene;                     // raw scene
		else if (v_TexCoord.x >= 0.5 && v_TexCoord.y < 0.5) outCol = vec3(edgeAt(v_TexCoord)); // edge map
		else if (v_TexCoord.x < 0.5 && v_TexCoord.y >= 0.5) outCol = nrm * 0.5 + 0.5;          // normals
		else
		{
			float ld = linearDepth(depth);
			float v = (depth > 0.9995) ? 0.05 : clamp(ld / 400.0, 0.0, 1.0);
			outCol = vec3(v);
		}
		color = vec4(outCol, 1.0);
		return;
	}

	// ---- Full-screen stylized output ----
	float day  = dayFactor();
	float warm = warmFactor();

	vec3 ro = u_CameraPos;
	vec3 rd = worldDir(v_TexCoord);

	vec3 outCol;
	if (depth > 0.9995)
	{
		// Sky: gradient + stars + sun/moon, then ink-dark cloud bands
		vec3 sky = skyColor(rd, day, warm);
		float cloud = cloudDensity(ro, rd, u_Time);
		vec3 cloudTint = mix(vec3(0.13, 0.15, 0.11), vec3(0.34, 0.36, 0.32), day);
		sky = mix(sky, cloudTint, clamp(cloud * 0.8, 0.0, 1.0));
		outCol = sky;
	}
	else
	{
		outCol = scene;

		// Ink outline (NMS edge on the toon buffer) — darkens into the ink color
		float e0 = edgeAt(v_TexCoord);
		vec2 dn = edgeDir(v_TexCoord);
		float eF = edgeAt(v_TexCoord + dn * texel);
		float eB = edgeAt(v_TexCoord - dn * texel);
		float edge = (e0 >= eF && e0 >= eB) ? e0 : 0.0;
		float edgeAmt = clamp(edge * u_OutlineStrength, 0.0, 1.0);
		outCol = mix(outCol, vec3(0.045, 0.045, 0.040), edgeAmt * 0.88);

		// Atmospheric depth fog — far geometry melts toward the sky-horizon tone
		float ld = linearDepth(depth);
		vec3 fogCol = mix(vec3(0.137, 0.153, 0.110), vec3(0.52, 0.56, 0.50), day);
		fogCol = mix(fogCol, vec3(0.90, 0.62, 0.38), warm * 0.35);
		float fogF = clamp(ld * 0.02 * u_FogStrength, 0.0, 0.55);
		outCol = mix(outCol, fogCol, fogF);
	}

	// Light dust motes floating in the air (over the whole frame)
	outCol += dustMotes(v_TexCoord, u_Time) * 0.55;

	// Paper grain
	float g = hash12(v_TexCoord * 900.0 + u_Time * 137.0) - 0.5;
	outCol += g * u_GrainAmount;

	// Color grade: contrast / saturation / brightness
	outCol = (outCol - 0.5) * u_Contrast + 0.5;
	float l = dot(outCol, vec3(0.299, 0.587, 0.114));
	outCol = mix(vec3(l), outCol, u_Saturation);
	outCol *= u_Brightness;

	// Vignette
	vec2 d = v_TexCoord - 0.5;
	float vig = 1.0 - dot(d, d) * 1.15;
	outCol *= mix(1.0, vig, u_Vignette);

	color = vec4(outCol, 1.0);
}
