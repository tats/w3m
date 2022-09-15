import sys
import pathlib
import re
from typing import NamedTuple, Union, List, Optional
from enum import Enum, auto
from termcolor import colored

CONTEXT = {
    'USE_M17N': True,
    'USE_UNICODE': True,
    # 'SIGWINCH': True,
    # 'USE_IMAGE': True,
    # 'USE_COLOR': True,
    # 'USE_ANSI_COLOR': True,
    # 'USE_MOUSE': False,
    # 'USE_GPM': False,
    # '__MINGW32_VERSION': False,
}

MACRO_PATTERN = re.compile(r'^#\s*(\S+)\s*(\S.*)?')


class Include(NamedTuple):
    value: str


class Define(NamedTuple):
    value: str


class If(NamedTuple):
    value: str

    def eval(self, context) -> Optional[bool]:
        if self.value == '0':
            return False
        return None


class Ifdef(NamedTuple):
    value: str

    def eval(self, context) -> Optional[bool]:
        return context.get(self.value)


class Ifndef(NamedTuple):
    value: str

    def eval(self, context) -> Optional[bool]:
        return None


class Elif(NamedTuple):
    value: str

    def eval(self, context) -> Optional[bool]:
        return None


class Else:
    def __str__(self) -> str:
        return 'Else'


class Endif:
    def __str__(self) -> str:
        return 'Endif'


def parse_macro(l: str) -> Union[None, Include, Define, If, Ifdef, Ifndef, Else, Elif, Endif]:
    m = MACRO_PATTERN.match(l)
    if m:
        match m.group(1):
            case 'include':
                return Include(m.group(2))
            case 'define':
                return Define(m.group(2))
            case 'if':
                return If(m.group(2))
            case 'ifndef':
                return Ifndef(m.group(2))
            case 'ifdef':
                return Ifdef(m.group(2))
            case 'elif':
                return Elif(m.group(2))
            case 'else':
                return Else()
            case 'endif':
                return Endif()


class MacroNode:
    def __init__(self, begin: int, begin_macro, prev=None) -> None:
        self.prev = prev
        self.begin = begin
        self.begin_macro = begin_macro
        self.end = begin
        self.end_macro = None
        self.children: List[MacroNode] = []
        self.result = None

    def close(self, end, end_macro):
        self.end = end
        if end_macro:
            self.end_macro = end_macro

    def get_prevs(self):
        prevs = []
        current = self
        while True:
            prevs.append(current.result)
            if not current.prev:
                break
            current = current.prev
        return prevs

    def eval(self, context):
        prevs = self.get_prevs()

        match self.begin_macro:
            case None:
                pass
            case If() | Ifndef() | Ifdef() as m:
                self.result = m.eval(context)
            case Elif() as m:
                if True in prevs:
                    pass
                else:
                    self.result = m.eval(context)
            case Else() as m:
                if any(prevs):
                    assert (self.result == None)
                    self.result = False
                elif any(prev == False for prev in prevs):
                    # has false
                    assert (self.result == None)
                    self.result = True
            case _:
                raise NotImplementedError()

        for child in self.children:
            child.eval(context)

    def print(self, indent=''):
        match self.result:
            case True:
                color = 'green'
            case False:
                color = 'red'
            case None:
                color = 'grey'
            case _:
                raise NotImplementedError()

        print(colored(f'{self.begin:04} ~ {self.end:04}:{indent}{self.begin_macro}', color))
        for child in self.children:
            child.print(indent + '  ')

    def apply(self, lines):
        removed = False
        # begin
        match self.result:
            case True | False:
                # lines[self.begin] = '// ' + lines[self.begin]
                lines[self.begin] = None
                removed = True
            case None:
                pass

        l = self.begin + 1
        for child in self.children:
            while l < child.begin:
                if self.result == False:
                    lines[l] = None
                l += 1
            child.apply(lines)
        while l < self.end:
            if self.result == False:
                lines[l] = None
            l += 1

        # end
        if self.end_macro and removed:
            # lines[self.end] = '// ' + lines[self.end]
            lines[self.end] = None


def main(path: pathlib.Path, debug=False):
    if path.suffix not in ['.h', '.c']:
        return
    lines = path.read_text().splitlines()
    root = MacroNode(0, None)
    stack = [root]
    for i, l in enumerate(lines):
        if l.startswith('#'):
            match parse_macro(l):
                case Include() as m:
                    pass
                    # print(f'[INCLUDE] => {m}')
                case Define() as m:
                    pass
                    # print(f'[DEFINE] => {m}')
                case If() | Ifndef() | Ifdef() as m:
                    node = MacroNode(i, m)
                    stack[-1].children.append(node)
                    stack.append(node)
                case Elif() | Else() as m:
                    stack[-1].close(i, None)
                    prev = stack.pop()
                    # new node
                    node = MacroNode(i, m, prev)
                    stack[-1].children.append(node)
                    stack.append(node)
                case Endif() as m:
                    stack[-1].close(i, m)
                    stack.pop()
                    pass
                case _:
                    print(l)
                    # print(l)
    assert (len(stack) == 1)
    root.close(len(lines) - 1, None)
    root.eval(CONTEXT)

    if debug:
        root.print()
    else:
        root.apply(lines)
        path.write_text(''.join(l + '\n' for l in lines if l != None))


if __name__ == '__main__':
    debug = False
    # debug = True
    for arg in sys.argv[1:]:
        main(pathlib.Path(arg), debug)