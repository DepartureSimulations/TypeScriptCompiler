// TODO: umcomment to see unused Symbols with generic func
/*
function test0() {
    const equal = <T>(lhs: T, rhs: T): boolean {
        return lhs === rhs;
    };
}
*/

function test1() {
    const equal = <T>(lhs: T, rhs: T): boolean {
        return lhs === rhs;
    };

    print(equal(1, 2));

    print(equal("asd", "asd"));
}

function test2() {
    let r = 1;

    const equal = <T, R>(lhs: T, rhs: R): boolean {
        print(r);
        return lhs === rhs;
    };

    print(equal(1, "asd"));
    
    print(equal("asd", "asd"));
}

function main() {
    test1();
    test2();
    print("done.");
}