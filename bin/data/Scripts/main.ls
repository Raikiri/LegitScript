void ColorPass(in vec3 in_color, out vec4 out_color)
{{
  void main()
  {
    out_color = vec4(in_color, 1.0f);
  }
}}

[rendergraph]
void RenderGraphMain()
{{
  void main()
  {
    Image img = GetImage(ivec2(128, 128), rgba8);
    Text("Mips count: " + img.GetMipsCount() + ". Mip 2 size: " + img.GetMip(0).GetSize());
    
    //Image img = GetImage(128, 128, rgba8);
    vec3 color = vec3(
      SliderFloat("R", 0.0f, 1.0f, 0.3f),
      SliderFloat("G", 0.0f, 1.0f, 0.4f),
      SliderFloat("B", 0.0f, 1.0f, 0.5f));
    Text("vec3: " + to_string(color) + " pi: " + to_string(3.1415f));
    Text("vec3: " + color + " pi: " + 3.1415f);
    ColorPass(
      color,
      GetSwapchainImage());
  }
}}