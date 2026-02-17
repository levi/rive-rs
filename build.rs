use std::env;
use std::fs;
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
        .file("cpp/provider_core.cpp")
        .files(runtime_sources)
        .compile("rive_rs_provider");
}
