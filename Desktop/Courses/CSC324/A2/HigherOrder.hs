module HigherOrder where

data BinaryTree a
  = Leaf a
  | Branch (BinaryTree a) a (BinaryTree a)
  deriving (Eq, Show)

foldTree :: (a -> b) -> (b -> a -> b -> b) -> BinaryTree a -> b
foldTree leaf branch tree = case tree of 
                            Leaf val -> leaf val
                            Branch left val right -> branch (foldTree leaf branch left) val (foldTree leaf branch right)

mapTree :: (a -> b) -> BinaryTree a -> BinaryTree b
mapTree f tree = foldTree (\x -> Leaf (f x)) (\left x right -> Branch left (f x) right) tree

preOrder :: BinaryTree a -> [a]
preOrder tree = foldTree (\x -> [x]) (\left x right -> [x] ++ left ++ right) tree 

inOrder :: BinaryTree a -> [a]
inOrder tree = foldTree (\x -> [x]) (\left x right -> left ++ [x] ++ right) tree

postOrder :: BinaryTree a -> [a]
postOrder tree = foldTree (\x -> [x]) (\left x right -> left ++ right ++ [x]) tree

newtype DList a = DList ([a] -> [a])

fromList :: [a] -> DList a
fromList xs = DList (xs ++)

toList :: DList a -> [a]
toList (DList f) = f []

empty :: DList a
empty = DList id

singleton :: a -> DList a
singleton x = DList (x :)

foldDList :: (a -> b -> b) -> b -> DList a -> b
foldDList f z d = foldr f z (toList d)

append :: DList a -> DList a -> DList a
append (DList f) (DList g) = DList (f . g) 

concatDList :: DList (DList a) -> DList a
concatDList list = foldDList append empty list

mapDList :: (a -> b) -> DList a -> DList b
mapDList f list = foldDList (\x acc -> append (singleton (f x)) acc) empty list 

filterDList :: (a -> Bool) -> DList a -> DList a
filterDList f list = foldDList (\x acc -> if f x then append (singleton x) acc else acc) empty list

inOrderFast :: BinaryTree a -> [a]
inOrderFast tree = toList (foldTree singleton (\left val right -> append left (append (singleton val) right)) tree)
