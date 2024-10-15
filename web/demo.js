
import Module from "./dist/LegitScriptWasm.js"
import { CreateEditor } from './editor.js'

Init(
  document.querySelector('#editor')
)

function CompileLegitScript(legitScriptCompiler, content) {
  const debugOutputEl = document.getElementById('compilation-result')

  try {
    const result = legitScriptCompiler.LegitScriptLoad(content)
    debugOutputEl.innerText = result
    debugOutputEl.style = 'border:2px solid green'
    return true
  } catch (e) {
    debugOutputEl.innerText = `${e.name} ${e.message}\n ${e.stack}`
    debugOutputEl.style = 'border:2px solid red'
    return false
  }
}

const initialContent = `
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
      Image img = GetImage(ivec2(128, 128), rgba8);
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
`

async function Init(editorEl) {
  const legitScriptCompiler = await Module();

  const editor = await CreateEditor(editorEl, initialContent)
  CompileLegitScript(legitScriptCompiler, editor.getModel().createSnapshot().read() || '')
  editor.getModel().onDidChangeContent((event) => {
    CompileLegitScript(legitScriptCompiler, editor.getModel().createSnapshot().read() || '')
  })

  // TODO: actually run frames
  // console.log('LegitScriptFrame', JSON.parse(legitScriptCompiler.LegitScriptFrame(1024, 512, Date.now())))
}
