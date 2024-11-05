import Module from "./dist/LegitScriptWasm.js"

Init(document.querySelector("#editor"), document.querySelector("results"))

function CompileLegitScript(legitScriptCompiler, content, imControls) {
  const debugOutputEl = document.querySelector("results result.compilation pre")
  try {
    const result = legitScriptCompiler.LegitScriptLoad(content)
    debugOutputEl.innerText = result

    if (result.includes('"error":')) {
      debugOutputEl.style = "border:2px solid yellow"
    } else {
      debugOutputEl.style = "border:2px solid green"
    }
    return true
  } catch (e) {
    debugOutputEl.innerText = `${e.name} ${e.message}\n ${e.stack}`
    debugOutputEl.style = "border:2px solid red"
    return false
  }
}

async function Init(editorEl, resultsEl) {
  const initialContent = editorEl.value
  const legitScriptCompiler = await Module()

  const controlEl = document.querySelector("inputs controls")
  const TextualOutputName = "@@textualOutput"
  const imControlContext = {}
  const imControls = {
    floatSlider(name, value, lo, hi) {
      const step = 0.01
      if (!imControlContext[name]) {
        const el = document.createElement("p")
        el.innerHTML = `
          ${name}
          <input type="range" min="${lo}" max="${hi}" value="${value}" step="${step}" />
          (<span class="value">${value}</span>)
        `
        imControlContext[name] = el
        controlEl.appendChild(el)
      }

      const el = imControlContext[name]
      const input = el.querySelector("input")
      value = parseFloat(input.value)
      el.querySelector(".value").innerText = value

      return value
    },
    intSlider(name, value, lo, hi) {
      const step = 1
      if (!imControlContext[name]) {
        const el = document.createElement("p")
        el.innerHTML = `
          ${name}
          <input type="range" min="${lo}" max="${hi}" value="${value}" step="${step}" />
          (<span class="value">${value}</span>)
        `
        imControlContext[name] = el
        controlEl.appendChild(el)
      }

      const el = imControlContext[name]
      const input = el.querySelector("input")
      value = parseInt(input.value)
      el.querySelector(".value").innerText = value

      return value
    },
    text(name, value) {
      if (!imControlContext[name]) {
        const el = document.createElement("pre")
        imControlContext[name] = el
        controlEl.appendChild(el)
      }

      const el = imControlContext[name]
      el.innerText = value + "\n"
    },
  }

  const compilationTimingEl = resultsEl.querySelector(".compilation .timings")

  function Compile() {
    const start = Date.now()
    CompileLegitScript(legitScriptCompiler, editorEl.value)
    compilationTimingEl.innerText = `${Date.now() - start}ms`
  }

  Compile()
  editorEl.addEventListener("keyup", Compile)

  const frameTimingEl = document.querySelector("results result.frame .timings")
  const frameOutputEl = document.querySelector("results result.frame pre")
  function Frame() {
    const start = performance.now()

    const frameResult = legitScriptCompiler.LegitScriptFrame(
      '[{"name": "@swapchain_size", "type": "uvec2", "value":{"x":512, "y":512}}]'
    )

    const result = JSON.parse(frameResult)
    if (result.context_requests) {
      const toRemove = new Set(Object.keys(imControlContext))
      let textualIndex = 0
      for (const request of result.context_requests) {
        if (request.type === "FloatRequest") {
          imControls.floatSlider(
            request.name,
            request.value,
            request.lo,
            request.hi
          )
          toRemove.delete(request.name)
        } else if (request.type === "IntRequest") {
          imControls.intSlider(
            request.name,
            request.value,
            request.lo,
            request.hi
          )
          toRemove.delete(request.name)
        } else if (request.type === "TextRequest") {
          const name = TextualOutputName + textualIndex++
          imControls.text(name, request.text)
          toRemove.delete(name)
        }
      }

      for (const key of toRemove) {
        controlEl.removeChild(imControlContext[key])
        delete imControlContext[key]
      }
    }

    frameTimingEl.innerText = `${(performance.now() - start).toFixed(2)}ms`
    if (frameResult !== frameOutputEl.innerText) {
      frameOutputEl.innerText = frameResult
    }
    setTimeout(Frame, 100)
  }

  Frame(0)
}
