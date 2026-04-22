lines = open('src/include/lexer.cpp').readlines()
new_lines = []
in_identifier = False
for line in lines:
    if line.strip().startswith('void Lexer::identifier()'):
        in_identifier = True
        new_lines.append(line)
        continue
    
    if in_identifier:
        # Check if we've reached the end of the method
        # This is a bit simplistic, but should work for this file
        if line.strip() == '}':
            # Append the new logic before the closing brace
            new_lines.append('    std::string text_str = extract_identifier_string();\n')
            new_lines.append('    // Check if this is a UTF-8 keyword\n')
            new_lines.append('    if (is_utf8_keyword(text_str)) {\n')
            new_lines.append('        std::string translated = translate_keyword(text_str, language_);\n')
            new_lines.append('        if (translated != text_str) {\n')
            new_lines.append('            if (translated == "z" || translated == "z.i") {\n')
            new_lines.append('                tokens_.emplace_back(TokenType::SYSTEM, std::string_view("z"), 0, line_);\n')
            new_lines.append('                return;\n')
            new_lines.append('            }\n')
            new_lines.append('            if (translated == "\x80") {\n')
            new_lines.append('                add_token(TokenType::TOK_CONST);\n')
            new_lines.append('                return;\n')
            new_lines.append('            }\n')
            new_lines.append('            if (translated.size() == 1 && is_keyword_char(translated[0])) {\n')
            new_lines.append('                add_token(keyword_type(translated[0]));\n')
            new_lines.append('                return;\n')
            new_lines.append('            }\n')
            new_lines.append('        }\n')
            new_lines.append('        add_token(TokenType::IDENTIFIER);\n')
            new_lines.append('        return;\n')
            new_lines.append('    }\n')
            new_lines.append('\n')
            new_lines.append('    std::string translated = translate_keyword(text_str, language_);\n')
            new_lines.append('    if (translated != text_str) {\n')
            new_lines.append('        if (translated == "z" || translated == "z.i") {\n')
            new_lines.append('            tokens_.emplace_back(TokenType::SYSTEM, std::string_view("z"), 0, line_);\n')
            new_lines.append('            return;\n')
            new_lines.append('        }\n')
            new_lines.append('        if (translated == "\x80") {\n')
            new_lines.append('            add_token(TokenType::TOK_CONST);\n')
            new_lines.append('            return;\n')
            new_lines.append('        }\n')
            new_lines.append('        if (translated.size() == 1 && text_str.size() > 1) {\n')
            new_lines.append('            char c = translated[0];\n')
            new_lines.append('            if (is_keyword_char(c)) {\n')
            new_lines.append('                add_token(keyword_type(c));\n')
            new_lines.append('                return;\n')
            new_lines.append('            }\n')
            new_lines.append('        }\n')
            new_lines.append('    }\n')
            new_lines.append('\n')
            new_lines.append('    if (text_str.size() == 1 && is_keyword_char(text_str[0])) {\n')
            new_lines.append('        add_token(keyword_type(text_str[0]));\n')
            new_lines.append('    } else {\n')
            new_lines.append('        add_token(TokenType::IDENTIFIER);\n')
            new_lines.append('    }\n')
            new_lines.append('}\n')
            in_identifier = False
        continue
    new_lines.append(line)

with open('src/include/lexer.cpp', 'w') as f:
    f.writelines(new_lines)
