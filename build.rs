use std::env;
use std::fs;
use std::io::Write;
use std::path::{Path, PathBuf};

fn collect_cpp_files(dir: &Path, out: &mut Vec<PathBuf>) {
    let entries =
        fs::read_dir(dir).unwrap_or_else(|err| panic!("failed to read {}: {err}", dir.display()));

    for entry in entries {
        let entry = entry
            .unwrap_or_else(|err| panic!("failed to read dir entry in {}: {err}", dir.display()));
        let path = entry.path();
        let file_type = entry
            .file_type()
            .unwrap_or_else(|err| panic!("failed to stat {}: {err}", path.display()));

        if file_type.is_dir() {
            collect_cpp_files(&path, out);
            continue;
        }

        if file_type.is_file()
            && path
                .extension()
                .and_then(|ext| ext.to_str())
                .is_some_and(|ext| ext == "cpp")
        {
            out.push(path);
        }
    }
}

fn is_ident_start(ch: char) -> bool {
    ch == '_' || ch.is_ascii_alphabetic()
}

fn is_ident_continue(ch: char) -> bool {
    ch == '_' || ch.is_ascii_alphanumeric()
}

fn collect_shader_files(dir: &Path) -> Vec<PathBuf> {
    let mut files = Vec::new();
    let entries =
        fs::read_dir(dir).unwrap_or_else(|err| panic!("failed to read {}: {err}", dir.display()));

    for entry in entries {
        let entry = entry
            .unwrap_or_else(|err| panic!("failed to read dir entry in {}: {err}", dir.display()));
        let path = entry.path();
        let is_shader = path
            .extension()
            .and_then(|ext| ext.to_str())
            .is_some_and(|ext| matches!(ext, "glsl" | "vert" | "frag"));
        if is_shader {
            files.push(path);
        }
    }

    files.sort();
    files
}

fn collect_exported_identifiers(source: &str) -> Vec<String> {
    let chars: Vec<char> = source.chars().collect();
    let mut exported = Vec::new();
    let mut i = 0usize;

    while i < chars.len() {
        if chars[i] == '@' && i + 1 < chars.len() && is_ident_start(chars[i + 1]) {
            let mut j = i + 2;
            while j < chars.len() && is_ident_continue(chars[j]) {
                j += 1;
            }
            exported.push(chars[i + 1..j].iter().collect::<String>());
            i = j;
            continue;
        }
        i += 1;
    }

    exported.sort();
    exported.dedup();
    exported
}

fn rewrite_shader_source(source: &str) -> String {
    let chars: Vec<char> = source.chars().collect();
    let mut out = String::with_capacity(source.len());
    let mut i = 0usize;

    while i < chars.len() {
        let ch = chars[i];
        if (ch == '@' || ch == '$') && i + 1 < chars.len() && is_ident_start(chars[i + 1]) {
            i += 1;
            continue;
        }
        out.push(ch);
        i += 1;
    }

    out
}

fn shader_symbol_name(path: &Path) -> String {
    let stem = path
        .file_stem()
        .and_then(|name| name.to_str())
        .unwrap_or_else(|| panic!("failed to derive shader stem from {}", path.display()));
    let ext = path
        .extension()
        .and_then(|name| name.to_str())
        .unwrap_or_else(|| panic!("failed to derive shader extension from {}", path.display()));

    if ext == "glsl" {
        stem.to_string()
    } else {
        format!("{stem}_{ext}")
    }
}

fn generate_shader_headers(shader_dir: &Path, generated_root: &Path) {
    fs::create_dir_all(generated_root)
        .unwrap_or_else(|err| panic!("failed to create {}: {err}", generated_root.display()));

    let shader_files = collect_shader_files(shader_dir);
    let mut all_exports = Vec::new();
    for shader_path in &shader_files {
        let source = fs::read_to_string(shader_path)
            .unwrap_or_else(|err| panic!("failed to read {}: {err}", shader_path.display()));
        all_exports.extend(collect_exported_identifiers(&source));
    }
    all_exports.sort();
    all_exports.dedup();

    for shader_path in shader_files {
        let basename = shader_path
            .file_name()
            .and_then(|name| name.to_str())
            .unwrap_or_else(|| panic!("invalid shader filename {}", shader_path.display()));
        let source = fs::read_to_string(&shader_path)
            .unwrap_or_else(|err| panic!("failed to read {}: {err}", shader_path.display()));
        let rewritten = rewrite_shader_source(&source);

        let exports_path = generated_root.join(format!("{basename}.exports.h"));
        let mut exports_file = fs::File::create(&exports_path)
            .unwrap_or_else(|err| panic!("failed to create {}: {err}", exports_path.display()));
        writeln!(exports_file, "#pragma once\n").expect("failed to write shader exports header");
        for ident in &all_exports {
            writeln!(exports_file, "#define GLSL_{ident} \"{ident}\"")
                .expect("failed to write GLSL export define");
            writeln!(exports_file, "#define GLSL_{ident}_raw {ident}")
                .expect("failed to write GLSL raw export define");
        }

        let hpp_path = generated_root.join(format!("{basename}.hpp"));
        let mut hpp_file = fs::File::create(&hpp_path)
            .unwrap_or_else(|err| panic!("failed to create {}: {err}", hpp_path.display()));
        let symbol = shader_symbol_name(&shader_path);
        writeln!(hpp_file, "#pragma once\n").expect("failed to write shader header");
        writeln!(hpp_file, "#include \"{basename}.exports.h\"\n")
            .expect("failed to write shader exports include");
        writeln!(hpp_file, "namespace rive {{").expect("failed to write namespace");
        writeln!(hpp_file, "namespace gpu {{").expect("failed to write namespace");
        writeln!(hpp_file, "namespace glsl {{").expect("failed to write namespace");
        writeln!(hpp_file, "const char {symbol}[] = R\"RIVE(")
            .expect("failed to write shader start");
        write!(hpp_file, "{rewritten}").expect("failed to write shader source");
        if !rewritten.ends_with('\n') {
            writeln!(hpp_file).expect("failed to terminate shader source");
        }
        writeln!(hpp_file, ")RIVE\";").expect("failed to write shader end");
        writeln!(hpp_file, "}} // namespace glsl").expect("failed to write namespace");
        writeln!(hpp_file, "}} // namespace gpu").expect("failed to write namespace");
        writeln!(hpp_file, "}} // namespace rive").expect("failed to write namespace");
    }
}

fn main() {
    println!("cargo:rerun-if-changed=include/rive_rs_abi.h");
    println!("cargo:rerun-if-changed=cpp/provider_core.cpp");
    println!("cargo:rerun-if-changed=submodules/rive-runtime/include");
    println!("cargo:rerun-if-changed=submodules/rive-runtime/src");

    if env::var_os("CARGO_FEATURE_RUNTIME_ABI_PROVIDER").is_none() {
        return;
    }

    let runtime_root = PathBuf::from("submodules/rive-runtime");
    let runtime_include = runtime_root.join("include");
    let runtime_dependencies = runtime_root.join("dependencies");
    let runtime_src = runtime_root.join("src");
    let target = env::var("TARGET").unwrap_or_default();
    let is_emscripten = target.contains("emscripten");

    let mut runtime_sources = Vec::new();
    collect_cpp_files(&runtime_src, &mut runtime_sources);
    runtime_sources.sort();

    let mut build = cc::Build::new();
    build
        .cpp(true)
        .include("include")
        .include(&runtime_include)
        .include(&runtime_dependencies)
        .define("_RIVE_INTERNAL_", None)
        .define("YOGA_EXPORT", "")
        .flag_if_supported("-std=c++17")
        .warnings(false)
        .file("cpp/provider_core.cpp");

    build.files(runtime_sources);

    if is_emscripten {
        println!("cargo:rerun-if-changed=submodules/rive-runtime/renderer/include");
        println!("cargo:rerun-if-changed=submodules/rive-runtime/renderer/src");
        println!("cargo:rerun-if-changed=submodules/rive-runtime/renderer/src/shaders");

        let renderer_root = runtime_root.join("renderer");
        let renderer_src = renderer_root.join("src");
        let generated_root =
            PathBuf::from(env::var("OUT_DIR").expect("OUT_DIR")).join("rive_renderer_generated");
        let generated_shaders = generated_root.join("generated").join("shaders");
        generate_shader_headers(&renderer_src.join("shaders"), &generated_shaders);

        let renderer_sources = vec![
            renderer_src.join("rive_renderer.cpp"),
            renderer_src.join("render_context.cpp"),
            renderer_src.join("rive_render_paint.cpp"),
            renderer_src.join("rive_render_path.cpp"),
            renderer_src.join("rive_render_image.cpp"),
            renderer_src.join("intersection_board.cpp"),
            renderer_src.join("draw.cpp"),
            renderer_src.join("gr_triangulator.cpp"),
            renderer_src.join("gradient.cpp"),
            renderer_src.join("sk_rectanizer_skyline.cpp"),
            renderer_src.join("gpu.cpp"),
            renderer_src.join("gpu_resource.cpp"),
            renderer_src.join("rive_render_factory.cpp"),
            renderer_src.join("render_context_helper_impl.cpp"),
            renderer_src.join("gl/gl_state.cpp"),
            renderer_src.join("gl/gl_utils.cpp"),
            renderer_src.join("gl/load_store_actions_ext.cpp"),
            renderer_src.join("gl/render_buffer_gl_impl.cpp"),
            renderer_src.join("gl/render_context_gl_impl.cpp"),
            renderer_src.join("gl/render_target_gl.cpp"),
            renderer_src.join("gl/pls_impl_webgl.cpp"),
        ];

        build
            .include(renderer_root.join("include"))
            .include(renderer_root.join("glad/include"))
            .include(&renderer_src)
            .include(&generated_root)
            .define("RIVE_WEBGL", None)
            .files(renderer_sources);
    }

    build.compile("rive_rs_provider");
}
