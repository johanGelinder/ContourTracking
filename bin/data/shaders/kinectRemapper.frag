#version 430

uniform     sampler2DRect   _depthTexture;

uniform		float			_nearClipping;
uniform		float			_farClipping;
uniform		int				_reverseColors;
uniform		int				_ignoreClipping;

in 			vec2			texCoordVarying;
in 			vec4    		pos;

out 		vec4 			outputColor;

void main()
{
	// uv for texture mapping
	vec3 depthColor = texture(_depthTexture, gl_FragCoord.xy).xyz;

	vec3 color = depthColor;

	float nearColor = 1.0;
	float farColor = 0.0;
	if(_reverseColors == 1){
		nearColor = 0.0;
		farColor = 1.0;
	}

	//convert clipping values from 16 bit (2^16 = 65536) to float:
	float nearClipping = _nearClipping/65536.0;
	float farClipping = _farClipping/65536.0;

	if (_ignoreClipping == 0 && color.r > farClipping || color.r < nearClipping) {
		if (_reverseColors == 1) color.r = 0.0;
		else color.r = 1.0;
	}
	else {
		float remappedColor = nearColor + (color.r - nearClipping) * (farColor - nearColor) / (farClipping - nearClipping);
		color.r = clamp(remappedColor,nearColor,farColor);
		color.r = remappedColor;
	}

	outputColor = vec4(color, 1.0);
}