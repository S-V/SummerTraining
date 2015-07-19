$input v_wpos, v_wnormal, v_view, v_normal, v_tangent, v_bitangent, v_texcoord0

#include "common.sh"

SAMPLER2D(s_texColor,  0);
SAMPLER2D(s_texNormal, 1);

uniform mat4 u_inverseViewMat;
uniform vec4 u_lightDirection;	// in view space

void main()
{
	vec3 normal;
	normal.xy = texture2D(s_texNormal, v_texcoord0).xy * 2.0 - 1.0;
	normal.z  = sqrt(1.0 - dot(normal.xy, normal.xy) );
//	normal = texture2D(s_texNormal, v_texcoord0).rgb * 2.0 - 1.0;

	mat3 tbn = mat3(
				normalize(v_tangent),
				normalize(v_bitangent),
				normalize(v_normal)
				);

	vec3 viewNormal = normalize(mul(tbn, normal) );

vec3 wnormal = normalize(mul(u_invView, vec4(viewNormal, 0.0) ).xyz);

//vec3 wnormal = normalize(mul(u_inverseViewMat, vec4(v_normal, 0.0) ).xyz);
//vec3 wnormal = v_wnormal;

vec3 u_lightDirWS = vec3(0,-1,0);
vec3 u_lightColor = vec3(0,1,0);
//vec3 u_lightDirWS = vec3(0,-1,0);
vec3 u_lightVecWS = -normalize(u_lightDirWS);

	//float NdotL = max(dot(viewNormal, u_lightDirection.xyz), 0.0);
	float NdotL = max(dot(wnormal, u_lightVecWS.xyz), 0.0);
	
	vec3 lightColor = u_lightColor.rgb * NdotL;
	
	vec3 diffuseColor = texture2D(s_texColor, v_texcoord0);

	gl_FragColor.xyz = diffuseColor * lightColor;
//	gl_FragColor.xyz = wnormal;
	gl_FragColor.w = 1.0;
	gl_FragColor = toGamma( gl_FragColor );
}
