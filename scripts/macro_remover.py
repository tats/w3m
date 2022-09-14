import sys
import pathlib
import re
from typing import NamedTuple, Union, List, Optional
from enum import Enum, auto
from termcolor import colored

CONTEXT = {
    'USE_M17N': True,
    'USE_UNICODE': True,
}

MACRO_PATTERN = re.compile(r'^#\s*(\S+)\s*(\S.*)?')


class Include(NamedTuple):
    value: str


class Define(NamedTuple):
    value: str


class If(NamedTuple):
    value: str

    def eval(self, context) -> Optional[bool]:
        return None


class Ifdef(NamedTuple):
    value: str

    def eval(self, context) -> Optional[bool]:
        if self.value in context:
            return True


class Ifndef(NamedTuple):
    value: str

    def eval(self, context) -> Optional[bool]:
        return None


class Elif(NamedTuple):
    value: str

    def eval(self, context) -> Optional[bool]:
        return None


class Else(NamedTuple):

    def eval(self, context) -> bool:
        return True


class Endif(NamedTuple):
    pass


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

    def close(self, end: int, end_macro):
        self.end = end
        self.end_macro = end_macro

    def eval(self, context):
        match self.begin_macro:
            case None:
                pass
            case If() | Ifndef() | Ifdef() | Elif() | Else() as m:
                has_true = False
                current = self
                while True:
                    if not current.prev:
                        break
                    current = current.prev
                    if current.result:
                        has_true = True
                        break
                if not has_true:
                    self.result = m.eval(context)
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


def main(path: pathlib.Path):
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
                    stack[-1].close(i, m)
                    stack.pop()
                    # new node
                    node = MacroNode(i, m)
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
    root.print()


if __name__ == '__main__':
    main(pathlib.Path(sys.argv[1]))
