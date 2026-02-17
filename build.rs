use std::collections::HashSet;
use std::env;
use std::fs;
use std::path::PathBuf;

fn main() {
    println!("cargo:rerun-if-changed=include/rive_rs_abi.h");
    println!("cargo:rerun-if-changed=cpp/provider_core.cpp");

    if env::var_os("CARGO_FEATURE_RUNTIME_ABI_PROVIDER").is_none() {
        return;
    }

    let header =
        fs::read_to_string("include/rive_rs_abi.h").expect("failed to read include/rive_rs_abi.h");
    let out_dir = PathBuf::from(env::var("OUT_DIR").expect("OUT_DIR not set"));
    let stub_path = out_dir.join("rive_rs_provider_stubs.cpp");

    let implemented = HashSet::from([
        "rive_rs_abi_version",
        "rive_rs_factory_default",
        "rive_rs_factory_ref",
        "rive_rs_factory_unref",
        "rive_rs_map_xy",
        "rive_rs_ptr_to_file_asset",
        "rive_rs_ptr_to_audio_asset",
        "rive_rs_ptr_to_image_asset",
        "rive_rs_ptr_to_font_asset",
    ]);

    let generated = generate_stubs(&header, &implemented);
    fs::write(&stub_path, generated).expect("failed to write generated stubs");

    cc::Build::new()
        .cpp(true)
        .file("cpp/provider_core.cpp")
        .file(&stub_path)
        .include("include")
        .include("submodules/rive-runtime/include")
        .flag_if_supported("-std=c++17")
        .warnings(false)
        .compile("rive_rs_provider");
}

fn generate_stubs(header: &str, implemented: &HashSet<&str>) -> String {
    let mut out = String::new();
    out.push_str("#include \"rive_rs_abi.h\"\n\n");
    out.push_str("extern \"C\" {\n");

    let mut collecting = false;
    let mut decl = String::new();

    for line in header.lines() {
        let trimmed = line.trim();
        if !collecting {
            if trimmed.starts_with("RIVE_RS_API ") {
                collecting = true;
                decl.push_str(trimmed);
                decl.push(' ');
                if trimmed.ends_with(';') {
                    append_stub(&mut out, &decl, implemented);
                    decl.clear();
                    collecting = false;
                }
            }
            continue;
        }

        decl.push_str(trimmed);
        decl.push(' ');
        if trimmed.ends_with(';') {
            append_stub(&mut out, &decl, implemented);
            decl.clear();
            collecting = false;
        }
    }

    out.push_str("} // extern \"C\"\n");
    out
}

fn append_stub(out: &mut String, declaration: &str, implemented: &HashSet<&str>) {
    let decl = declaration.trim().trim_end_matches(';').trim();
    let no_api = decl.replacen("RIVE_RS_API", "", 1);
    let no_api = no_api.trim();

    let Some(open_paren) = no_api.find('(') else {
        return;
    };
    let prefix = &no_api[..open_paren];
    let Some(name) = prefix.split_whitespace().last() else {
        return;
    };

    if implemented.contains(name) {
        return;
    }

    let return_type = prefix
        .strip_suffix(name)
        .unwrap_or(prefix)
        .trim()
        .to_string();

    out.push_str(no_api);
    out.push_str(" {\n");
    if !return_type.is_empty() && return_type != "void" {
        out.push_str("  ");
        out.push_str(default_return_stmt(&return_type));
        out.push('\n');
    }
    out.push_str("}\n\n");
}

fn default_return_stmt(return_type: &str) -> &'static str {
    if return_type == "rive_rs_status" {
        return "return RIVE_RS_STATUS_UNSUPPORTED;";
    }
    if return_type == "bool" {
        return "return false;";
    }
    if return_type == "float" {
        return "return 0.0f;";
    }
    if return_type == "double" {
        return "return 0.0;";
    }
    if return_type.contains('*') {
        return "return nullptr;";
    }
    if return_type.contains("size_t")
        || return_type.contains("uint")
        || return_type.contains("int")
        || return_type.contains("uintptr_t")
    {
        return "return 0;";
    }
    "return {};"
}
