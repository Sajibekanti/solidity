contract C {
	function dyn(uint x) public returns (bytes memory a) {
		assembly {
			mstore(0, 0x20)
			mstore(0x20, 0x21)
			return(0, x)
		}
	}
	function f(uint x) public returns (bool) {
		this.dyn(x);
		return true;
	}
}
// ====
// EVMVersion: >=byzantium
// revertStrings: debug
// ----
// f(uint256): 0x60 -> FAILURE, hex"08c379a0", 0x20, 46, "ABI memory decoding error: inval", "id data length"
