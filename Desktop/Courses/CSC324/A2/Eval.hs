module Eval where

import BasicExpr
import Expr

eval :: Env -> Expr -> Value
eval _ (ENum n) = VInt n 
eval _ (EBool b) = VBool b 
eval env (EVar x) = Eval.lookup x env
eval env (EBin op e1 e2) = evalOp op v1 v2
  where v1 = eval env e1
        v2 = eval env e2
eval env (ELet x e1 e2) = eval env1 e2 
  where v1 = eval env e1
        env1 = (x, v1) : env 
eval env (ELam x e) = VClos env x e
eval env (EApp e1 e2) = 
  let f = eval env e1
      arg = eval env e2
  in case f of
    VClos frozenv x body -> eval ((x, arg):frozenv) body
    _                    -> error "SyntaxError: Application of non-functions."
eval env (EIf cond body elseBody) = case eval env cond of
  VBool True -> eval env body 
  VBool False -> eval env elseBody
  _ -> error "TypeError: Condition must evaluate to type VBool."
eval env (EMLetRec bindings expr) =
  let bound = map fst bindings
      boundenv = map (\(x, e) -> (x, eval (boundenv ++ env) e)) bindings
  in eval (boundenv ++ env) expr

lookup :: Id -> Env -> Value
lookup x [] = error ("NameError: No variable named " ++ x ++ " was found in the environment.")
lookup x ((key, value):rest) 
  = if x == key then value
    else Eval.lookup x rest

evalOp :: Binop -> Value -> Value -> Value
evalOp Add (VInt v1) (VInt v2) = VInt (v1 + v2)
evalOp Sub (VInt v1) (VInt v2) = VInt (v1 - v2)
evalOp Mul (VInt v1) (VInt v2) = VInt (v1 * v2)
evalOp Eq (VInt v1) (VInt v2) = VBool (v1 == v2)
evalOp Eq (VBool b1) (VBool b2) = VBool (b1 == b2)
evalOp And (VBool b1) (VBool b2) = VBool (b1 && b2)
evalOp Or (VBool b1) (VBool b2) = VBool (b1 || b2)
evalOp _ _ _ = error "ArithmeticError: Invalid operand types for specified binary operation."

data EvaluateResult = EvaluateOk Value | EvaluateUndefinedVariables
  deriving (Eq, Show)

evaluate :: Expr -> EvaluateResult
evaluate e = 
  let freeIds = exprFreeIds e 
  in if (freeIds == []) then EvaluateOk (eval [] e) else EvaluateUndefinedVariables
