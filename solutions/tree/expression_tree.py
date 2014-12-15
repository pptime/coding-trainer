# infix expression representation to postfix expression representation
# postfix or prefix representation is easier for parsing and tree construction


def infix_to_postfix(infixexpr):
    prec = dict()
    prec["*"] = 3
    prec["/"] = 3
    prec["+"] = 2
    prec["-"] = 2
    prec["("] = 1
    op_stack = list()
    postfix_list = []
    token_list = infixexpr.split()

    for token in token_list:
        if token in "ABCDEFGHIJKLMNOPQRSTUVWXYZ" or token in "0123456789":
            postfix_list.append(token)
        elif token == '(':
            op_stack.append(token)
        elif token == ')':
            top_token = op_stack.pop()
            while top_token != '(':
                postfix_list.append(top_token)
                top_token = op_stack.pop()
        else:
            while op_stack and \
                    (prec[op_stack[-1]] >= prec[token]):
                postfix_list.append(op_stack.pop())
            op_stack.append(token)

    while op_stack:
        postfix_list.append(op_stack.pop())
    return " ".join(postfix_list)


print(infix_to_postfix("A * B + C * D"))
print(infix_to_postfix("( A + B ) * C - ( D - E ) * ( F + G )"))


class Operand:

    def __init__(self, sym):
        self.sym = sym

    def __str__(self):
        return str(self.sym)

    def __repr__(self):
        return str(self)


class Operator:

    def __init__(self, sym):
        self.sym = sym
        self.left = None
        self.right = None

    def __str__(self):
        return str(self.sym) + ':[{left}(left), {right}(right)]'.format(left=str(self.left),
                                                                       right=str(self.right))

    def __repr__(self):
        return str(self)


def construct(stack):
    if not stack:
        return None
    sym = stack.pop()
    if sym in "+-*/":
        operator = Operator(sym)
        operator.right = construct(stack)
        operator.left = construct(stack)
        return operator
    elif sym in "ABCDEFGHIJKLMNOPQRSTUVWXYZ":
        operand = Operand(sym)
        return operand
    else:
        raise Exception('Invalid symbol: {}'.format(sym))


def parse_postfix_expr(postfix_expr):
    stack = list([x for x in postfix_expr if x!=' '])
    root = Operator(stack.pop())
    root.right = construct(stack)
    root.left = construct(stack)
    return root

print(parse_postfix_expr(infix_to_postfix("A * B + C * D")))
print(parse_postfix_expr(infix_to_postfix("( A + B ) * C - ( D - E ) * ( F + G )")))
