# Question 1: Trace and Predict-the-Output Verification

This document provides the exact actual outputs and step-by-step traces for the two predict-the-output programs in Q1, complete with tree drawings and deletion case analyses.

---

## 1. Predict the Output 1 (`q1_predict1.c`)

### Source Code Context
- Storage: Array `T = "ABCDE.F"`, size `N = 7`.
- Formula: For a node at index `i`, its left child is `2*i + 1` and right child is `2*i + 2`.
- A period `'.'` denotes an empty/absent node.

### Tree Structure Derived from Array
- **Index 0**: `T[0] = 'A'` (Root)
  - Left child (`2*0 + 1 = 1`): `T[1] = 'B'`
    - Left child (`2*1 + 1 = 3`): `T[3] = 'D'` (Leaf: children 7 and 8 out of bounds)
    - Right child (`2*1 + 2 = 4`): `T[4] = 'E'` (Leaf: children 9 and 10 out of bounds)
  - Right child (`2*0 + 2 = 2`): `T[2] = 'C'`
    - Left child (`2*2 + 1 = 5`): `T[5] = '.'` (Empty subtree)
    - Right child (`2*2 + 2 = 6`): `T[6] = 'F'` (Leaf: children 13 and 14 out of bounds)

```
        A
       / \
      B   C
     / \   \
    D   E   F
```

### Trace of the Four Traversals
1. **Preorder (`pre(0)`: Root -> Left -> Right)**:
   - Visit A -> print `A`
   - Visit left child B -> print `B`
     - Visit left child D -> print `D`
     - Visit right child E -> print `E`
   - Visit right child C -> print `C`
     - Left child is `.` (returns immediately)
     - Visit right child F -> print `F`
   - **Result**: `ABDECF`

2. **Inorder (`in(0)`: Left -> Root -> Right)**:
   - Left subtree of A:
     - Left child of B: D -> print `D`
     - Root B -> print `B`
     - Right child of B: E -> print `E`
   - Root A -> print `A`
   - Right subtree of A:
     - Left child of C is `.` (returns)
     - Root C -> print `C`
     - Right child of C: F -> print `F`
   - **Result**: `DBEACF`

3. **Postorder (`post(0)`: Left -> Right -> Root)**:
   - Left subtree of A:
     - Left child D -> print `D`
     - Right child E -> print `E`
     - Root B -> print `B`
   - Right subtree of A:
     - Left child is `.`
     - Right child F -> print `F`
     - Root C -> print `C`
   - Root A -> print `A`
   - **Result**: `DEBFCA`

4. **Loop Traversal (`for (int i = 0; i < N; i++) if (T[i] != '.') printf("%c", T[i]);`)**:
   - Iterates directly across the array indices: `0('A'), 1('B'), 2('C'), 3('D'), 4('E'), 6('F')`.
   - **Name of Traversal**: **Level-Order Traversal (Breadth-First Search)**.
   - **Result**: `ABCDEF`

### Actual Output of `q1_predict1.c`
```text
ABDECF DBEACF DEBFCA ABCDEF
```

---

## 2. Predict the Output 2 (`q1_predict2.c`)

### Initial Insertion Sequence: `50, 30, 70, 20, 40, 60, 80, 65`

1. Insert 50:
   ```
     50
   ```
2. Insert 30 (30 < 50):
   ```
       50
      /
     30
   ```
3. Insert 70 (70 > 50):
   ```
       50
      /  \
     30   70
   ```
4. Insert 20 (20 < 50, 20 < 30):
   ```
         50
        /  \
       30   70
      /
     20
   ```
5. Insert 40 (40 < 50, 40 > 30):
   ```
         50
        /  \
       30   70
      /  \
     20   40
   ```
6. Insert 60 (60 > 50, 60 < 70):
   ```
         50
        /  \
       30   70
      /  \  /
     20  40 60
   ```
7. Insert 80 (80 > 50, 80 > 70):
   ```
         50
        /  \
       30   70
      /  \  / \
     20  40 60 80
   ```
8. Insert 65 (65 > 50, 65 < 70, 65 > 60):
   ```
         50
        /  \
       30   70
      /  \  / \
     20  40 60 80
             \
             65
   ```

---

### Step-by-Step Deletions

#### Step 1: `del(t, 20)`
- **Target Node**: 20
- **Deletion Case**: **Case 1 – Leaf Node (0 children)**.
- **Action**: Node 20 has `l == NULL` and `r == NULL`. The pointer `30->l` is replaced with `NULL`, and node 20 is freed.
- **Tree State after Step 1**:
  ```
        50
       /  \
      30   70
       \   / \
       40 60  80
            \
            65
  ```

#### Step 2: `del(t, 30)`
- **Target Node**: 30
- **Deletion Case**: **Case 2 – Node with One Child (right child 40)**.
- **Action**: Node 30 has `l == NULL` but `r != NULL` (child is 40). Node 30 is replaced directly by its only child 40. Pointer `50->l` becomes 40, and node 30 is freed.
- **Tree State after Step 2**:
  ```
        50
       /  \
      40   70
           / \
          60  80
            \
            65
  ```

#### Step 3: `del(t, 50)`
- **Target Node**: 50 (Root)
- **Deletion Case**: **Case 3 – Node with Two Children (left child 40, right child 70)**.
- **Action**:
  1. Locate the **inorder successor** (the smallest key in the right subtree `t->r = 70`).
  2. Starting at 70, follow `s->l`: node 60 has `s->l == NULL`. Thus, **inorder successor is 60**.
  3. Copy key 60 to the root: `t->k = 60`.
  4. Recursively delete key 60 from the right subtree `t->r` via `del(70, 60)`.
  5. In `del(70, 60)`, node 60 has **one child** (right child 65, which is Case 2). Node 60 is freed, and node 65 becomes the left child of 70 (`70->l = 65`).
- **Tree State after Step 3**:
  ```
        60
       /  \
      40   70
          /  \
         65   80
  ```

---

### Preorder Traversal of Final Tree (`pre(t)`)
- Visit Root: `60 `
- Visit Left subtree: `40 `
- Visit Right subtree (Root 70): `70 `
  - Visit Left of 70: `65 `
  - Visit Right of 70: `80 `

### Actual Output of `q1_predict2.c`
```text
60 40 70 65 80 
```
