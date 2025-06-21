use std::{i32, mem::swap};

struct Fighter {
    //name: &'static str,
    hp: i32,
    damage: i32,
    armor: i32,
}

struct Item {
    //name: String,
    cost: i32,
    damage: i32,
    armor: i32,
}

struct Items {
    weapons: Vec<Item>,
    armor: Vec<Item>,
    rings: Vec<Item>,
}

const ITEMS: &'static str = "
    Weapons:    Cost  Damage  Armor
    Dagger        8     4       0
    Shortsword   10     5       0
    Warhammer    25     6       0
    Longsword    40     7       0
    Greataxe     74     8       0

    Armor:      Cost  Damage  Armor
    Leather      13     0       1
    Chainmail    31     0       2
    Splintmail   53     0       3
    Bandedmail   75     0       4
    Platemail   102     0       5

    Rings:      Cost  Damage  Armor
    Damage+1    25     1       0
    Damage+2    50     2       0
    Damage+3   100     3       0
    Defense+1   20     0       1
    Defense+2   40     0       2
    Defense+3   80     0       3";

fn main() {
    let items = parse_items(ITEMS);
    part1(&items);
    part2(&items);
}

fn do_rings(equipped: Vec<&Item>, rings: &Vec<Item>, min_cost: &mut i32) {
    for ring_count in 0..=2 {
        if ring_count == 0 {
            fight(equipped.clone(), min_cost);
        } else if ring_count == 1 {
            for ring in rings {
                let mut equipped = equipped.clone();
                equipped.push(ring);
                fight(equipped, min_cost);
            }
        } else {
            for i in 0..(rings.len()-1) {
                for j in (i+1)..rings.len() {
                    let mut equipped = equipped.clone();
                    equipped.push(&rings[i]);
                    equipped.push(&rings[j]);
                    fight(equipped, min_cost);
                }
            }
        }
    }
}

fn part1(items: &Items) {
    let mut min_cost = i32::MAX;
    for weapon in &items.weapons {
        for armor_count in 0..=1 {
            if armor_count == 0 {
                do_rings(vec![weapon], &items.rings, &mut min_cost);
            } else {
                for armor in &items.armor {
                    do_rings(vec![weapon, armor], &items.rings, &mut min_cost);
                }
            }
        }
    }

    println!("min_cost = {min_cost}");
    assert_eq!(min_cost, 78);
}

fn part2(items: &Items) {
    let mut max_cost = i32::MIN;
    for weapon in &items.weapons {
        for armor_count in 0..=1 {
            if armor_count == 0 {
                for ring_count in 0..=2 {
                    if ring_count == 0 {
                        fight2(&vec![weapon], &mut max_cost);
                    } else if ring_count == 1 {
                        for ring in &items.rings {
                            fight2(&vec![weapon, ring], &mut max_cost);
                        }
                    } else {
                        for i in 0..items.rings.len()-1 {
                            for j in i..items.rings.len() {
                                fight2(&vec![weapon, &items.rings[i], &items.rings[j]], &mut max_cost);
                            }
                        }
                    }
                }                
            } else {
                for armor in &items.armor {
                    for ring_count in 0..=2 {
                        if ring_count == 0 {
                            fight2(&vec![weapon, armor], &mut max_cost);
                        } else if ring_count == 1 {
                            for ring in &items.rings {
                                fight2(&vec![weapon, armor, ring], &mut max_cost);
                            }
                        } else {
                            for i in 0..(items.rings.len()-1) {
                                for j in (i+1)..items.rings.len() {
                                    fight2(&vec![weapon, armor, &items.rings[i], &items.rings[j]], &mut max_cost);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    println!("max_cost = {max_cost}");
    assert_eq!(max_cost, 148);
}

fn fight(items: Vec<&Item>, min_cost: &mut i32) {
    let mut total_damage = 0;
    let mut total_armor = 0;
    let mut total_cost = 0;

    for item in items {
        total_damage += item.damage;
        total_armor += item.armor;
        total_cost += item.cost;
    }
    let mut player = Fighter { /*name: "player",*/ hp: 100, damage: total_damage, armor: total_armor };
    let mut boss = Fighter { /*name: "boss",*/ hp: 104, damage: 8, armor: 1 };
    simulate_battle(&mut player, &mut boss);
    if player.hp > 0 {
        if total_cost < *min_cost {
            *min_cost = total_cost;
        }
    }
}

fn fight2(items: &Vec<&Item>, max_cost: &mut i32) {
    let mut total_damage = 0;
    let mut total_armor = 0;
    let mut total_cost = 0;

    for item in items {
        total_damage += item.damage;
        total_armor += item.armor;
        total_cost += item.cost;
    }
    let mut player = Fighter { /*name: "player",*/ hp: 100, damage: total_damage, armor: total_armor };
    let mut boss = Fighter { /*name: "boss",*/ hp: 104, damage: 8, armor: 1 };
    simulate_battle(&mut player, &mut boss);
    if player.hp <= 0 {
        if total_cost > *max_cost {
            *max_cost = total_cost;
        }
    }
}

fn parse_items(input: &str) -> Items {
    let mut weapons: Vec<Item> = Vec::new();
    let mut armor: Vec<Item> = Vec::new();
    let mut rings: Vec<Item> = Vec::new();

    let mut lines_iter = input.lines();

    while let Some(line) = lines_iter.next() {
        if let Some(item_class) = line.split_ascii_whitespace().next() {
            match item_class {
                "Weapons:" => fill_items(&mut weapons, &mut lines_iter),
                "Armor:" => fill_items(&mut armor, &mut lines_iter),
                "Rings:" => fill_items(&mut rings, &mut lines_iter),
                _ => panic!("Unknown item class: {item_class}"),
            };
        }
    }

    return Items{ weapons, armor, rings };
}

fn fill_items(items: &mut Vec<Item>, lines_iter: &mut std::str::Lines) {
    while let Some(line) = lines_iter.next() {
        let mut words_iter = line.split_ascii_whitespace();
        if let Some(_name) = words_iter.next() {
            let cost: i32 = words_iter.next().unwrap().parse().unwrap();
            let damage: i32 = words_iter.next().unwrap().parse().unwrap();
            let armor: i32 = words_iter.next().unwrap().parse().unwrap();
            items.push(Item{ /*name: name.to_string(),*/ cost, damage, armor });
        } else {
            break;
        }
    }
}

fn simulate_battle(player: &mut Fighter, boss: &mut Fighter) {
    let mut attacker = player;
    let mut defender = boss;
    while attacker.hp > 0 && defender.hp > 0 {
        let attack_damage = std::cmp::max(attacker.damage - defender.armor, 1);
        defender.hp -= attack_damage;
        //println!("The {} deals {} damage; the {} goes down to {} hit points.", attacker.name, attack_damage, defender.name, defender.hp);
        swap(&mut attacker, &mut defender);
    }
}
