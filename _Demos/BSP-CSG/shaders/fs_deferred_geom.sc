$input v_wpos, v_wnormal, v_view, v_normal, v_tangent, v_bitangent, v_texcoord0

//based on
/*
 * Copyright 2011-2015 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#include "common.sh"

SAMPLER2D(s_texColor,  0);
SAMPLER2D(s_texNormal, 1);

uniform mat4 u_inverseViewMat;

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

	viewNormal = v_normal;

//	vec3 wnormal = normalize(mul(u_inverseViewMat, vec4(viewNormal, 0.0) ).xyz);
//	wnormal = normalize(mul(u_invView, vec4(v_normal, 0.0) ).xyz);

	gl_FragData[0] = texture2D(s_texColor, v_texcoord0);
	gl_FragData[1] = vec4(encodeNormalUint(viewNormal), 1.0);
}
