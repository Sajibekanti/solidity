{
    function a() {}
    function f() { mstore(0, 1) }
    function g() { sstore(0, 1) }
    function h() { let x := msize() }
    function i() { let z := mload(0) }
}
// ====
// Dialect: ewasm
// ----
// DeclarationError: (41-47): Function not found.
// DeclarationError: (75-81): Function not found.
// DeclarationError: (118-123): Function not found.
// DeclarationError: (109-125): Variable count mismatch: 1 variables and 0 values.
// DeclarationError: (156-161): Function not found.
// DeclarationError: (147-164): Variable count mismatch: 1 variables and 0 values.
