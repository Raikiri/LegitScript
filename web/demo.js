
import Module from "./dist/LegitScriptWasm.js"

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

async function Init(editorEl) {
  const initialContent = editorEl.value
  const legitScriptCompiler = await Module();
  CompileLegitScript(legitScriptCompiler, initialContent)
  editorEl.addEventListener('keyup', (e) => {
    CompileLegitScript(legitScriptCompiler, editorEl.value)
  })

  // TODO: actually run frames
  // console.log('LegitScriptFrame', JSON.parse(legitScriptCompiler.LegitScriptFrame(1024, 512, Date.now())))
}
