lines = open('src/include/vm.h').readlines()
new_lines = []
for line in lines:
    if 'Value stack_[STACK_MAX];' in line:
        new_lines.append('    std::unique_ptr<Value[]> stack_;\n')
        new_lines.append('    size_t stack_capacity_ = STACK_MAX;\n')
    elif 'Value *stack_ptr_ = stack_;' in line:
        new_lines.append('    Value *stack_ptr_;\n')
    else:
        new_lines.append(line)
with open('src/include/vm.h', 'w') as f:
    f.writelines(new_lines)
