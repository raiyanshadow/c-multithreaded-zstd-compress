module BasicExpr where

import Expr

exprSize :: Expr -> Int
exprSize (ENum _) = 1 
exprSize (EBin _ e1 e2) = 1 + (exprSize e1) + (exprSize e2)
exprSize (EVar _) = 1
exprSize (ELet _ e1 e2) = 1 + (exprSize e1) + (exprSize e2)
exprSize (ELam _ expr) = 1 + (exprSize expr)
exprSize (EApp e1 e2) = 1 + (exprSize e1) + (exprSize e2)
exprSize (EBool _) = 1
exprSize (EIf cond body elseBody) = 1 + (exprSize cond) + (exprSize body) + (exprSize elseBody)
exprSize (EMLetRec bindings expr) = 1 + (sum (map (exprSize . snd) bindings)) + (exprSize expr)

isNotBound :: Id -> [Id] -> Bool
isNotBound _ [] = True
isNotBound x (y:rest) = if x == y then False else (isNotBound x rest)

exprFreeIds :: Expr -> [Id]
exprFreeIds (ENum _) = []
exprFreeIds (EBin _ e1 e2) = (exprFreeIds e1) ++ (exprFreeIds e2)
exprFreeIds (EVar x) = [x]
exprFreeIds (ELet x e1 e2) = (exprFreeIds e1) ++ (filter (/= x) (exprFreeIds e2))
exprFreeIds (ELam x expr) = filter (/= x) (exprFreeIds expr)
exprFreeIds (EApp e1 e2) = (exprFreeIds e1) ++ (exprFreeIds e2)
exprFreeIds (EBool _) = []
exprFreeIds (EIf cond body elseBody) = (exprFreeIds cond) ++ (exprFreeIds body) ++ (exprFreeIds elseBody)
exprFreeIds (EMLetRec bindings expr) = 
    let bound = map fst bindings
        allVars = concat (map (exprFreeIds . snd) bindings) ++ (exprFreeIds expr)
    in filter (\x -> isNotBound x bound) allVars

exprReplace :: (Expr -> Expr) -> Expr -> Expr
exprReplace f e = case e of
    ENum n                  -> f (ENum n)
    EVar x                  -> f (EVar x)
    EBool b                 -> f (EBool b)
    EBin op e1 e2           -> f (EBin op (exprReplace f e1) (exprReplace f e2))
    ELet x e1 e2            -> f (ELet x (exprReplace f e1) (exprReplace f e2))
    ELam x expr             -> f (ELam x (exprReplace f expr))
    EApp e1 e2              -> f (EApp (exprReplace f e1) (exprReplace f e2))
    EIf cond body elseBody  -> f (EIf (exprReplace f cond) (exprReplace f body) (exprReplace f elseBody))
    EMLetRec bindings expr  -> f (EMLetRec (map (\(x, y) -> (x, exprReplace f y)) bindings) (exprReplace f expr))

exprConstantFoldBool :: Expr -> Expr
exprConstantFoldBool expr = exprReplace fold expr
    where
        fold (EBin And (EBool b1) (EBool b2)) = EBool (b1 && b2)
        fold (EBin Or  (EBool b1) (EBool b2)) = EBool (b1 || b2)
        fold (EBin Eq (ENum n1) (ENum n2)) = EBool (n1 == n2)
        fold e = e
