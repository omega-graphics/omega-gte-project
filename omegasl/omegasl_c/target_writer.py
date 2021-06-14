from .target import *
import io

class TargetWriter(object):

    outputStr : io.TextIOWrapper

    target: Target

    resourceQueue : "list[ast.AnnAssign]"

    declared_vars: "list[str]"

    def __init__(self,t:Target):
        self.target = t 
        self.resourceQueue = []
        self.outputStr = t.out
        return 

    def resourceExists(self,name:str):
        for r in self.resourceQueue:
            res_node:ast.Name = r.target
            if res_node.id == name:
                return r
        return None

    def writeShaderResourceDecl(self,expr:ast.AnnAssign):
        res_name:ast.Name = expr.target
        if len(self.resourceQueue) > 0:
            if self.resourceExists(res_name.id) is not None:
                raise RuntimeError("Resources must have unique names")

        if self.target.type == TargetType.HLSL:
            self.outputStr.write(self.target.annotationToString(expr))
        self.resourceQueue.append(expr)
        return

    def writeDecl(self,decl:ast.stmt,funcContext:str):
        """
        Writes a Decl to Output Stream
        (Must be defined only in a function or shader)
        """
        # VarDecl
        if isinstance(decl,ast.AnnAssign):
            var_ty = decl.annotation
            var_name = decl.target

            if not isinstance(var_name,ast.Name):
                raise RuntimeError("AST NAME EXPECTED!!")

            if not isinstance(var_ty,ast.Name):
                raise RuntimeError("AST NAME EXPECTED!!")
            
            if not is_standard_type(var_ty.id):
                self.target.writeIdentifier(var_ty.id)
            else:
                self.target.writeType(var_ty.id)

            self.outputStr.write(" ")
            self.target.writeIdentifier(var_name.id)
            if decl.value is not None:
                self.outputStr.write(" = ")
                self.writeExpr(decl.value)
            return
        # CondDecl
        elif isinstance(decl,ast.If):
            return
        # AssignDecl
        elif isinstance(decl,ast.Assign):
            self.writeExpr(decl.targets[0])
            self.outputStr.write(" = ")
            self.writeExpr(decl.value)
        # ReturnDecl
        elif isinstance(decl,ast.Return):
            
            if funcContext == "fragment":
                if decl.value is None:
                    raise RuntimeError("Fragment shader must return a value (Specifically a float4)")

                self.target.writeFragmentShaderReturn()
                self.writeExpr(decl.value)
            elif funcContext == "vertex":
                self.target.writeVertexShaderReturn()
                if decl.value is None:
                    raise RuntimeError("Vertex shader must return a value!")
                self.writeExpr(decl.value)
            return
        return

    def writeExpr(self,expr:ast.expr):
        """
        Writes an AST Expr to the Output Stream.
        (Does not include VarDecls)
        """
        if isinstance(expr,ast.Constant):
            self.outputStr.write(str(expr.value))
            return
        elif isinstance(expr,ast.Name):
            self.target.writeIdentifier(expr.id)
        # Math Operations
        elif isinstance(expr,ast.BinOp):

            self.writeExpr(expr.left)
            self.target.writeOperator(expr.op)
            self.writeExpr(expr.right)

            return
        # Bool Operations
        elif isinstance(expr,ast.BoolOp):
            self.target.writeLogicOperator(expr.op)
        # Function Invocation
        elif isinstance(expr,ast.Call):
            func_n:ast.Name = expr.func
            self.outputStr.write(func_n.id)
            self.outputStr.write("(")
            for i in range(len(expr.args)):
                if i > 0:
                    self.outputStr.write(",")
                self.writeExpr(expr.args[i])
            self.outputStr.write(")")
            return
        
        elif isinstance(expr,ast.List):
            # Write C Array if Metal

            if self.target.type == TargetType.METAL:
                self.outputStr.write("{")
                for i in range(len(expr.elts)):
                    if i > 0:
                        self.outputStr.write(", ")
                    self.writeExpr(expr.elts[i])
                self.outputStr.write("}")
            else:
                # Write Matrix Struct Constructor Call.
                l = len(expr.elts)
                if l == 1:
                    self.outputStr.write(f"float(")
                else:
                    self.outputStr.write(f"float{l}(")
                
                for i in range(l):
                    if i > 0:
                        self.outputStr.write(", ")
                    self.writeExpr(expr.elts[i])
                self.outputStr.write(")")
                return

        elif isinstance(expr,ast.Attribute):
            self.writeExpr(expr.value)
            self.outputStr.write(".")
            self.outputStr.write(expr.attr)
        elif isinstance(expr,ast.Subscript):
            self.writeExpr(expr.value)
            self.target.out.write("[")
            t = expr.slice
            if isinstance(t,ast.Tuple):
                for i in range(len(t.elts)):
                    if i > 0:
                         self.target.out.write(", ")
                    self.writeExpr(t.elts[i])
            else:
                self.writeExpr(expr.slice)
            self.target.out.write("]")
        # return

    def writeStruct(self,struct:ast.ClassDef):
        decorator_count = len(struct.decorator_list)
        if decorator_count == 0 or decorator_count > 1:
            raise RuntimeError("All classes defined in OmegaSLPy must be annotated with the 'struct' decorator")

        class_name = struct.name
        self.target.writeStructDecl(class_name)
        self.target.beginBlock()

        for prop in struct.body:
            if isinstance(prop,ast.AnnAssign):
                prop_name:ast.Name = prop.target
                prop_type:ast.Name = prop.annotation
                self.target.writeStructPropDecl(prop_name.id,prop_type.id)
            else:
                raise RuntimeError("Only Variable Decls are allowed")
            # print(prop)
        self.target.endBlock()

        return
    def writeShaderFunction(self,func:ast.FunctionDef):
        decorator_count = len(func.decorator_list)
        if decorator_count  == 0 or decorator_count > 1:
            raise RuntimeError("A Shader Function must have one and ONLY one type")

        func_decor:ast.Call = func.decorator_list[0]
        shader_type:str = func_decor.func.id

        params:"dict[str,str]" = {}

        for a in func_decor.args:
            if not isinstance(a,ast.Name):
                raise RuntimeError("Only identifiers are allowed in this context")

            arg:ast.Name = a
            res = self.resourceExists(arg.id)
            if res is None:
                raise RuntimeError(f"Resource {arg.id} does not exist")
            if self.target.type == TargetType.METAL:
                t = self.target.annotationToString(res)
                params[arg.id] = t

        ret:ast.Name = func.returns
        if ret is None:
            raise RuntimeError("OmegaSLPy requires that all shaders must be type annotated")

        
        for arg in func.args.args:
            if arg.annotation is None:
                raise RuntimeError("OmegaSLPy requires that all shaders must be type annotated")
            params[arg.arg] = arg.annotation.id
        
        if shader_type == "vertex":
            self.target.writeVertexShaderDecl(func.name,type_str=ret.id,params=params)
        elif shader_type == "fragment":
            if params.get("input") is None:
                raise RuntimeError("There must be only 1 argument to a fragment function")
            self.target.writeFragmentShaderDecl(func.name,type_str=ret.id,params=params)
        self.target.beginBlock()

        for stmt in func.body:
            self.outputStr.write("  ")
            if isinstance(stmt,ast.Expr):
                self.writeExpr(stmt.value)
            else:
                self.writeDecl(stmt,shader_type)
            self.outputStr.write(";\n")
        self.target.endBlock()
