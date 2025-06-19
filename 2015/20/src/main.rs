use std::collections::HashMap;

fn part1() {
    let mut i = 0;
    let mut presents = 0;
    while presents < 29_000_000 {
        i += 1;
        presents = 0;
        for j in 1..(i+1) {
            if i % j == 0 {
                presents += j * 10;
            }
        }
        if i % 1000 == 0 {
            println!("House {i} got {presents} presents.");
        }
    }
    println!("House {i} got {presents} presents.");
}

fn part2() {
    let mut m = HashMap::new();
    let mut i = 0;
    let mut presents = 0;
    while presents < 29_000_000 {
        i += 1;
        presents = 0;
        for j in 1..(i+1) {
            if i % j == 0 {
                let entry = m.entry(j).or_insert(0);
                if *entry < 50 {
                    *entry += 1;
                    presents += j * 11;
                }
            }
        }
        if i % 1000 == 0 {
            println!("House {i} got {presents} presents.");
        }
    }
    println!("House {i} got {presents} presents.");
}

fn main() {
    //part1();
    part2();
}
