$input v_texcoord0

#include "common.sh"

SAMPLER2D(s_normal, 0);
SAMPLER2D(s_depth,  1);

uniform vec4 u_lightDirection;
uniform vec4 u_lightColor;

vec2 blinn(vec3 _lightDir, vec3 _normal, vec3 _viewDir)
{
	float ndotl = dot(_normal, _lightDir);
	vec3 reflected = _lightDir - 2.0*ndotl*_normal; // reflect(_lightDir, _normal);
	float rdotv = dot(reflected, _viewDir);
	return vec2(ndotl, rdotv);
}

float fresnel(float _ndotl, float _bias, float _pow)
{
	float facing = (1.0 - _ndotl);
	return max(_bias + (1.0 - _bias) * pow(facing, _pow), 0.0);
}

vec4 lit(float _ndotl, float _rdotv, float _m)
{
	float diff = max(0.0, _ndotl);
	float spec = step(0.0, _ndotl) * max(0.0, _rdotv * _m);
	return vec4(1.0, diff, spec, 1.0);
}

float toClipSpaceDepth(float _depthTextureZ)
{
#if BGFX_SHADER_LANGUAGE_HLSL
	return _depthTextureZ;
#else
	return _depthTextureZ * 2.0 - 1.0;
#endif // BGFX_SHADER_LANGUAGE_HLSL
}

vec3 clipToWorld(mat4 _invViewProj, vec3 _clipPos)
{
	vec4 wpos = mul(_invViewProj, vec4(_clipPos, 1.0) );
	return wpos.xyz / wpos.w;
}

void main()
{
	vec3  normal      = decodeNormalUint(texture2D(s_normal, v_texcoord0).xyz);
	float deviceDepth = texture2D(s_depth,  v_texcoord0).x;
	float depth       = toClipSpaceDepth(deviceDepth);

	vec3 clip = vec3(v_texcoord0 * 2.0 - 1.0, depth);
#if BGFX_SHADER_LANGUAGE_HLSL
	clip.y = -clip.y;
#endif // BGFX_SHADER_LANGUAGE_HLSL
	vec3 wpos = clipToWorld(u_invViewProj, clip);

	vec3 eyePos = u_invView[3].xyz;
	vec3 viewDir = normalize(wpos - eyePos);
	vec2 bln = blinn(u_lightDirection, normal, viewDir);
	vec4 lc = lit(bln.x, bln.y, 1.0);
	vec3 rgb = u_lightColor.xyz * saturate(lc.y);

	gl_FragColor.xyz = toGamma(rgb.xyz);
	gl_FragColor.w = 1.0;
}
