#version 430

uniform     sampler2DRect   _depthTexture;
uniform     sampler2DRect   _subtractionTexture;

uniform		float			_minReturnColor;
uniform		float			_maxReturnColor;
uniform		int				_clampColor;
uniform		int				_setSubtractionActive;

in 			vec2			texCoordVarying;
in 			vec4    		pos;

out 		vec4 			outputColor;

void main()
{
	// uv for texture mapping
	vec3 depthColor = texture(_depthTexture, gl_FragCoord.xy).xyz;
	vec3 subtractionColor = texture(_subtractionTexture, gl_FragCoord.xy).xyz;

	vec3 color = depthColor;

	float minReturnColor = _minReturnColor / 255.0;
	float maxReturnColor = _maxReturnColor / 255.0;

	if (_setSubtractionActive == 1) {
		// Subtract the two texture
		color = vec3(depthColor - subtractionColor);
		
		// Cut off colors that is under or over "minReturnColor" and "maxReturnColor"
		if (color.r > maxReturnColor &&
			color.r < minReturnColor) {
			color.r = 0.0;
		}
	}

	// Clamp the color between min and max to 1.0, set color out of range to 0.0
	if (_clampColor == 1) {
		if (color.r > minReturnColor &&
			color.r < maxReturnColor) {
			color.r = 1.0;
		}
		if (color.r != 1.0)
			color.r = 0.0;
	}

	outputColor = vec4(color, 1.0);
}