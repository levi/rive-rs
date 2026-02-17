use regex::Regex;
use std::collections::HashSet;
use std::fs;
use std::path::PathBuf;

fn parse_declared_symbols(header: &str) -> Vec<String> {
    let mut symbols = Vec::new();
    let mut collecting = false;
    let mut declaration = String::new();

    for line in header.lines() {
        let trimmed = line.trim();

        if !collecting {
            if trimmed.starts_with("RIVE_RS_API ") {
                collecting = true;
                declaration.push_str(trimmed);
                declaration.push(' ');
                if trimmed.ends_with(';') {
                    collecting = false;
                    if let Some(symbol) = symbol_from_declaration(&declaration) {
                        symbols.push(symbol);
                    }
                    declaration.clear();
                }
            }
            continue;
        }

        declaration.push_str(trimmed);
        declaration.push(' ');
        if trimmed.ends_with(';') {
            collecting = false;
            if let Some(symbol) = symbol_from_declaration(&declaration) {
                symbols.push(symbol);
            }
            declaration.clear();
        }
    }

    symbols
}

fn symbol_from_declaration(declaration: &str) -> Option<String> {
    let decl = declaration.trim().trim_end_matches(';').trim();
    let no_api = decl.strip_prefix("RIVE_RS_API")?.trim();
    let prefix = no_api.split('(').next()?.trim();
    let name = prefix.split_whitespace().last()?;
    Some(name.to_string())
}

fn parse_defined_symbols(provider_source: &str) -> HashSet<String> {
    // Match C ABI function definitions, including multiline signatures.
    let pattern =
        Regex::new(r"(?s)\n(?:[A-Za-z_][\w\s\*]*?)\b(rive_rs_[A-Za-z0-9_]+)\s*\([^;]*?\)\s*\{")
            .expect("valid provider symbol regex");

    pattern
        .captures_iter(provider_source)
        .map(|caps| caps[1].to_string())
        .collect()
}

#[test]
fn provider_implements_every_declared_abi_symbol() {
    let root = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
    let header_path = root.join("include/rive_rs_abi.h");
    let provider_path = root.join("cpp/provider_core.cpp");

    let header = fs::read_to_string(&header_path)
        .unwrap_or_else(|err| panic!("failed to read {}: {err}", header_path.display()));
    let provider_source = fs::read_to_string(&provider_path)
        .unwrap_or_else(|err| panic!("failed to read {}: {err}", provider_path.display()));

    let declared = parse_declared_symbols(&header);
    let defined = parse_defined_symbols(&provider_source);

    let declared_set: HashSet<String> = declared.into_iter().collect();

    let mut missing: Vec<String> = declared_set
        .iter()
        .filter(|symbol| !defined.contains(*symbol))
        .cloned()
        .collect();
    missing.sort();

    let mut extra: Vec<String> = defined
        .iter()
        .filter(|symbol| !declared_set.contains(*symbol))
        .cloned()
        .collect();
    extra.sort();

    assert!(
        missing.is_empty(),
        "provider is missing ABI symbols:\n{}",
        missing.join("\n")
    );
    assert!(
        extra.is_empty(),
        "provider defines non-contract ABI symbols:\n{}",
        extra.join("\n")
    );
}
