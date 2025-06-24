use std::cmp;

#[derive(Debug)]
struct State {
    is_player_turn: bool,
    turn: i32,

    hp: i32,
    armor: i32,
    mana: i32,
    total_mana_spent: i32,

    boss_hp: i32,

    effect_shield: i32,
    effect_poison: i32,
    effect_recharge: i32,
}

const MISSILE_COST: i32 = 53;
const DRAIN_COST: i32 = 73;
const SHIELD_COST: i32 = 113;
const POISON_COST: i32 = 173;
const RECHARGE_COST: i32 = 229;

const SHIELD_DURATION: i32 = 6;
const SHIELD_ARMOR: i32 = 7;

const POISON_DURATION: i32 = 6;
const POISON_DAMAGE: i32 = 3;

const RECHARGE_DURATION: i32 = 5;
const RECHARGE_MANA: i32 = 101;

fn apply_effects(state: &State) -> State {
    let mut result = State{ ..*state };

    if result.effect_shield > 0 {
        result.effect_shield -= 1;
        if result.effect_shield == 0 {
            result.armor -= SHIELD_ARMOR;
            assert_eq!(result.armor, 0);
        }
    }

    if result.effect_poison > 0 {
        result.effect_poison -= 1;
        result.boss_hp -= POISON_DAMAGE;
    }

    if result.effect_recharge > 0 {
        result.effect_recharge -= 1;
        result.mana += RECHARGE_MANA;
    }

    result
}

fn get_new_states(state: &State, boss_damage: i32, is_hard: bool) -> Vec<State> {
    assert!(state.hp > 0);
    assert!(state.boss_hp > 0);
    assert!(state.effect_poison >= 0);
    assert!(state.effect_shield >= 0);
    assert!(state.effect_recharge >= 0);

    let mut new_states = Vec::new();

    let state = if is_hard && state.is_player_turn {
        State{ hp: state.hp - 1, ..*state }
    } else {
        State{ ..*state }
    };

    if state.hp <= 0 {
        return new_states;
    }

    let state = apply_effects(&state);
    
    if state.is_player_turn {
        if state.boss_hp > 0 {
            // Generate new state for each possible spell
            for i in 0..5 {
                match i {
                    0 => {
                        // missile
                        if state.mana >= MISSILE_COST {
                            new_states.push(State{
                                turn: state.turn + 1,
                                is_player_turn: false,
                                mana: state.mana - MISSILE_COST,
                                total_mana_spent: state.total_mana_spent + MISSILE_COST,
                                boss_hp: state.boss_hp - 4,
                                ..state
                            });
                        }
                    }
                    1 => {
                        // drain
                        if state.mana >= DRAIN_COST {
                            new_states.push(State{
                                turn: state.turn + 1,
                                is_player_turn: false,
                                mana: state.mana - DRAIN_COST,
                                total_mana_spent: state.total_mana_spent + DRAIN_COST,
                                hp: state.hp + 2,
                                boss_hp: state.boss_hp - 2,
                                ..state
                            });
                        }
                    }
                    2 => {
                        // shield
                        if state.mana >= SHIELD_COST && state.effect_shield == 0 {
                            assert_eq!(state.armor, 0);
                            new_states.push(State{
                                turn: state.turn + 1,
                                is_player_turn: false,
                                mana: state.mana - SHIELD_COST,
                                total_mana_spent: state.total_mana_spent + SHIELD_COST,
                                effect_shield: SHIELD_DURATION,
                                armor: state.armor + 7,
                                ..state
                            });
                        }
                    }
                    3 => {
                        // poison
                        if state.mana >= POISON_COST && state.effect_poison == 0 {
                            new_states.push(State{
                                turn: state.turn + 1,
                                is_player_turn: false,
                                mana: state.mana - POISON_COST,
                                total_mana_spent: state.total_mana_spent + POISON_COST,
                                effect_poison: POISON_DURATION,
                                ..state
                            });
                        }
                    }
                    _ => {
                        // recharge
                        assert_eq!(i, 4);
                        if state.mana >= RECHARGE_COST && state.effect_recharge == 0 {
                            new_states.push(State{
                                turn: state.turn + 1,
                                is_player_turn: false,
                                mana: state.mana - RECHARGE_COST,
                                total_mana_spent: state.total_mana_spent + RECHARGE_COST,
                                effect_recharge: RECHARGE_DURATION,
                                ..state
                            });
                        }
                    }
                }
            }
        } else {
            // Boss is dead after applying effects
            new_states.push(state);
        }
    } else {
        // Boss attacks the player
        let damage = cmp::max(1, boss_damage - state.armor);
        let new_hp = state.hp - damage;
        let new_state = State{
            turn: state.turn + 1,
            is_player_turn: true, hp: new_hp, ..state
        };
        new_states.push(new_state);
    }

    new_states
}

fn find_min_mana_to_win(is_hard: bool) -> i32 {
    let boss_hp = 55;
    let boss_damage = 8;
    let initial_state = State{
        turn: 0,
        is_player_turn: true, hp: 50, armor: 0, mana: 500, boss_hp, total_mana_spent: 0,
        effect_poison: 0, effect_recharge: 0, effect_shield: 0,
    };
    let mut states = Vec::new();
    states.push(initial_state);
    let mut min_mana_spent = i32::MAX;
    while let Some(state) = states.pop() {
        if state.boss_hp <= 0 {
            min_mana_spent = cmp::min(min_mana_spent, state.total_mana_spent);
        } else if state.hp > 0 {
            if state.total_mana_spent < min_mana_spent {
                let mut new_states = get_new_states(&state, boss_damage, is_hard);
                states.append(&mut new_states);
            }
        }
    }
    min_mana_spent
}

fn main() {
    let answer1 = find_min_mana_to_win(false);
    dbg!(answer1);
    let answer2 = find_min_mana_to_win(true);
    dbg!(answer2);
    // not 1309
}
