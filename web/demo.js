import Module from "./dist/LegitScriptWasm.js"

(async () => {
  const m = await Module();

  const result = m.LegitScriptLoad(`
    void ColorPass(in float r, in float g, in float b, out vec4 out_color)
    {{
      void main()
      {
        out_color = vec4(r, g, b + 0.5f, 1.0f);
      }
    }}

    void RenderGraphMain()
    {{
      void main()
      {
        Image img = GetImage(128, 128, rgba8);
        ColorPass(
          SliderFloat("R", 0.0f, 1.0f) + 0.5f,
          SliderFloat("G", 0.0f, 1.0f),
          SliderFloat("B", 0.0f, 1.0f),
          GetSwapchainImage());
        int a = SliderInt("Int param", -42, 42, 5);
        float b = SliderFloat("Float param", -42.0f, 42.0f);
        //float e = SliderFloat("Float param", -42.0f, 42.0f);
        Text("script int: " + formatInt(a) + " float: " + formatFloat(b));
      }
    }}
  `)
  console.log('LegitScriptLoad result', JSON.parse(result))

  console.log('LegitScriptFrame', JSON.parse(m.LegitScriptFrame(1024, 512, Date.now())))

})();
