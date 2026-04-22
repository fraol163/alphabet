lines = open('src/include/lexer.cpp').readlines()
new_lines = []
for i, line in enumerate(lines):
    if i == 166:
        new_lines.append('    size_t newline_pos = header_source.find("\n", close_pos);\n')
        new_lines.append('    if (newline_pos != std::string_view::npos) {\n')
        new_lines.append('        current_ = current_ + newline_pos + 1;\n')
        new_lines.append('        start_ = current_;\n')
        new_lines.append('        line_ = 2;\n')
        new_lines.append('        column_ = 0;\n')
        new_lines.append('    }\n')
        new_lines.append('}\n')
        # Skip the original corrupted lines 167-175
        break
    else:
        new_lines.append(line)
with open('src/include/lexer.cpp', 'w') as f:
    f.writelines(new_lines)
