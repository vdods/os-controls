#version 120

// These are the inputs from the vertex shader to the fragment shader, and must appear identically there.
varying vec3 out_normal;
varying vec3 out_position;
varying vec2 out_tex_coord;

uniform vec3 light_position;      // The position of the (single) light for diffuse reflectance.  It is assumed to be white.
uniform vec4 diffuse_color;       // The color for diffuse lighting.
uniform vec4 ambient_color;       // The color for ambient lighting.
uniform float ambient_proportion; // Lighting color for each fragment is determined by linearly interpolating 
                                  // between and ambient lighting colors.  This variable is in the range [0,1].
                                  // A value of 0 or 1 specifies that the color is entirely diffuse or ambient,
                                  // respectively.
uniform bool use_texture;
uniform sampler2D texture;

void main() {
  // Compute diffuse brightness: a value in [0,1] giving the proportion of reflected light from the light source.
  vec3 surface_normal = normalize(out_normal);
  vec3 light_dir = normalize(light_position - out_position);
  float diffuse_brightness = max(0.0, dot(light_dir, surface_normal));
  
  // Blend the ambient and diffuse lighting.
  gl_FragColor = ambient_proportion*ambient_color + (1.0-ambient_factor)*diffuse_brightness*diffuse_color;
  // If texturing is enabled, include its influence in the color.
  if (use_texture) {
    // The fragment color is used as a color mask, hence the multiplication.
    gl_FragColor *= texture2D(texture, out_tex_coord);
  }
}
