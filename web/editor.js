
const LocalStorageVersion = '7'
const EditorStateLocalStorageKey = 'shader-editor'

export async function CreateEditor(editorEl, initialContent) {
  require.config({ paths: { vs: 'dependencies/monaco-editor/min/vs' } });
  const monaco = await new Promise(resolve => {
    require(['vs/editor/editor.main'], resolve)
  })

  const editor = monaco.editor.create(editorEl, {
    value: initialContent || '',
    language: 'c',
    minimap: {
      enabled: false
    },
    tabSize: 2,
    automaticLayout: true,
    theme: 'vs-dark'
  })
  editor.focus()
  return editor
}

export function PersistEditorState(content, editor) {
  const viewState = editor.saveViewState()
  // DEBUG: wire up persistence via local storage
  window.localStorage.setItem(EditorStateLocalStorageKey, JSON.stringify({
    viewState,
    content: content
  }))
}