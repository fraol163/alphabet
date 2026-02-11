import sys
from bytecode import OpCode

class AlphabetObject:
    def __init__(self, class_info):
        self.class_info = class_info
        self.fields = {}

class CallFrame:
    def __init__(self, bytecode, ip=0, locals=None):
        self.bytecode = bytecode
        self.ip = ip
        self.locals = locals if locals is not None else {}
        self.try_stack = [] 

class VM:
    def __init__(self, program=None):
        self.classes = program["classes"] if program else {}
        self.stack = []
        self.globals = {}
        self.frames = []
        if program:
            self.init_program(program)

    def init_program(self, program):
        self.classes.update(program["classes"])
        for class_name, class_info in program["classes"].items():
            class_info["static_values"] = {field_name: 0 for field_name in class_info["static_fields"]}
        for class_name, class_info in program["classes"].items():
            if class_info["static_init"]:
                init_frame = CallFrame(class_info["static_init"])
                self.frames.append(init_frame)
                self.run_loop()
        if program["main"]:
            self.frames.append(CallFrame(program["main"]))

    def run(self, program=None):
        if program:
            self.init_program(program)
        self.run_loop()

    def run_loop(self):
        start_frame_count = len(self.frames)
        while self.frames:
            frame = self.frames[-1]
            if frame.ip >= len(frame.bytecode):
                self.frames.pop()
                if len(self.frames) < start_frame_count:
                    break
                continue
            op, operand = frame.bytecode[frame.ip]
            frame.ip += 1
            if op == OpCode.PUSH_CONST:
                self.stack.append(operand)
            elif op == OpCode.LOAD_VAR:
                if operand in frame.locals:
                    self.stack.append(frame.locals[operand])
                else:
                    self.stack.append(self.globals.get(operand, 0))
            elif op == OpCode.STORE_VAR:
                val = self.stack[-1]
                if operand in frame.locals:
                    frame.locals[operand] = val
                else:
                    self.globals[operand] = val
            elif op == OpCode.POP:
                self.stack.pop()
            elif op == OpCode.ADD:
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a + b)
            elif op == OpCode.SUB:
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a - b)
            elif op == OpCode.GT:
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a > b)
            elif op == OpCode.JUMP_IF_FALSE:
                cond = self.stack.pop()
                if not cond: frame.ip = operand
            elif op == OpCode.JUMP:
                frame.ip = operand
            elif op == OpCode.PRINT:
                val = self.stack.pop()
                _obj = self.stack.pop()
                print(val)
                self.stack.append(None)
            elif op == OpCode.NEW:
                class_name = operand
                if class_name in self.classes:
                    obj = AlphabetObject(self.classes[class_name])
                    self.stack.append(obj)
                else:
                    raise Exception(f"Unknown class: {class_name}")
            elif op == OpCode.CALL:
                method_name, arg_count = operand
                args = [self.stack.pop() for _ in range(arg_count)]
                obj = self.stack.pop()
                caller_class = None
                if "this" in frame.locals:
                    caller_class = frame.locals["this"].class_info["name"]
                if isinstance(obj, AlphabetObject):
                    method_bytecode = self.lookup_method(obj.class_info, method_name, caller_class)
                    new_locals = {"this": obj}
                    self.frames.append(CallFrame(method_bytecode, locals=new_locals))
                elif isinstance(obj, int): 
                    class_info = next((c for c in self.classes.values() if c["id"] == obj), None)
                    if not class_info:
                        raise Exception(f"Unknown class ID: {obj}")
                    if method_name in class_info["static_methods"]:
                        method_bytecode = class_info["static_methods"][method_name]
                        self.frames.append(CallFrame(method_bytecode, locals={}))
                    else:
                        raise Exception(f"Static method {method_name} not found in {class_info['name']}")
                else:
                    if obj == 'SYSTEM_Z':
                        if method_name == 'o': 
                            val = args[0]
                            print(val)
                            self.stack.append(None)
                        elif method_name == 'i': 
                            val = input()
                            try:
                                val = float(val)
                            except: pass
                            self.stack.append(val)
                        elif method_name == 't': 
                            self.throw_exception("Custom Error 15")
                        elif method_name == 'f': 
                            with open(args[0], 'r') as f:
                                self.stack.append(f.read())
                        continue
                    raise Exception(f"Cannot call method {method_name} on non-object {obj}")
            elif op == OpCode.RET:
                val = self.stack.pop()
                self.frames.pop()
                if self.frames:
                    self.stack.append(val)
            elif op == OpCode.SETUP_TRY:
                frame.try_stack.append((operand, len(self.stack)))
            elif op == OpCode.POP_TRY:
                frame.try_stack.pop()
            elif op == OpCode.THROW:
                val = self.stack.pop()
                self.throw_exception(val)
            elif op == OpCode.HALT:
                break
            elif op == OpCode.GET_STATIC:
                field_name = operand
                class_id = self.stack.pop()
                class_info = next((c for c in self.classes.values() if c["id"] == class_id), None)
                if not class_info: raise Exception(f"Unknown class ID: {class_id}")
                self.stack.append(class_info["static_values"].get(field_name, 0))
            elif op == OpCode.SET_STATIC:
                field_name = operand
                val = self.stack.pop()
                class_id = self.stack.pop()
                class_info = next((c for c in self.classes.values() if c["id"] == class_id), None)
                if not class_info: raise Exception(f"Unknown class ID: {class_id}")
                class_info["static_values"][field_name] = val
                self.stack.append(val)
            elif op == OpCode.LOAD_FIELD:
                field_name = operand
                obj = self.stack.pop()
                if not isinstance(obj, AlphabetObject): raise Exception("LOAD_FIELD on non-object")
                field_node = obj.class_info["fields"].get(field_name)
                if field_node and field_node.visibility and field_node.visibility.type == TokenType.PRIVATE:
                    caller_class = frame.locals.get("this").class_info["name"] if "this" in frame.locals else None
                    if caller_class != obj.class_info["name"]:
                        raise Exception(f"Private field {field_name} is not accessible.")
                self.stack.append(obj.fields.get(field_name, 0))
            elif op == OpCode.STORE_FIELD:
                field_name = operand
                val = self.stack.pop()
                obj = self.stack.pop()
                if not isinstance(obj, AlphabetObject): raise Exception("STORE_FIELD on non-object")
                field_node = obj.class_info["fields"].get(field_name)
                if field_node and field_node.visibility and field_node.visibility.type == TokenType.PRIVATE:
                    caller_class = frame.locals.get("this").class_info["name"] if "this" in frame.locals else None
                    if caller_class != obj.class_info["name"]:
                        raise Exception(f"Private field {field_name} is not accessible.")
                obj.fields[field_name] = val
                self.stack.append(val)
            elif op == OpCode.BUILD_LIST:
                count = operand
                items = [self.stack.pop() for _ in range(count)]
                items.reverse()
                self.stack.append(items)
            elif op == OpCode.BUILD_MAP:
                count = operand
                m = {}
                for _ in range(count):
                    v = self.stack.pop()
                    k = self.stack.pop()
                    m[k] = v
                self.stack.append(m)
            elif op == OpCode.LOAD_INDEX:
                idx = self.stack.pop()
                obj = self.stack.pop()
                if isinstance(obj, list) and isinstance(idx, float):
                    idx = int(idx)
                self.stack.append(obj[idx])
            elif op == OpCode.STORE_INDEX:
                val = self.stack.pop()
                idx = self.stack.pop()
                obj = self.stack.pop()
                if isinstance(obj, list) and isinstance(idx, float):
                    idx = int(idx)
                obj[idx] = val
                self.stack.append(val)

    def throw_exception(self, message):
        while self.frames:
            frame = self.frames[-1]
            if frame.try_stack:
                handler_ip, stack_depth = frame.try_stack.pop()
                while len(self.stack) > stack_depth:
                    self.stack.pop()
                self.stack.append(message)
                frame.ip = handler_ip
                return
            else:
                self.frames.pop()
        sys.exit(1)

    def lookup_method(self, class_info, method_name, caller_class=None):
        if method_name in class_info["methods"]:
            method_node = class_info["method_nodes"][method_name]
            if method_node.visibility and method_node.visibility.type == TokenType.PRIVATE:
                if caller_class != class_info["name"]:
                    raise Exception(f"Private method {method_name} not found.")
            return class_info["methods"][method_name]
        if class_info["superclass"]:
            super_class = self.classes[class_info["superclass"]]
            return self.lookup_method(super_class, method_name, caller_class)
        raise Exception(f"Method {method_name} not found.")
