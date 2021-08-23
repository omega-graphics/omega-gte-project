from enum import Enum



class Stmt:
    def __init__(self):
        pass 
    line:int
    startCol:int
    endCol:int


class Decl:
    def __init__(self):
        pass


class Block(Stmt):
    def __init__(self):
        self.body = []
        pass 
    body:"list[Stmt]"



class FuncDecl(Decl):
    def __init__(self):
        pass 
    name:str 
    b:Block




class RoutineType(Enum):
    RENDER = 1
    KERNEL = 2,



class RoutineDecl(Decl):
    def __init__(self):
        pass
    name:str
    b:Block



class ResourceType(Enum):
    BUFFER = 0
    CONSTBUFFER = 1
    TEXTURE1D = 2
    TEXTURE2D = 3
    TEXTURE3D = 4



class ResourceDecl(Decl):
    def __init__(self):
        pass 
    name:str
    location:int
    t:ResourceType



class ParsedSourceFile :
    def __init__(self):
        self.body = []
        pass 
    name:str 
    body:"list[Decl]"
    