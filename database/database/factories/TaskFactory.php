<?php

use Faker\Generator as Faker;

$factory->define(App\Task::class, function (Faker $faker) {
    return [
        'title' => $faker->sentence,
        'description' => $faker->sentence,
        'price' => $faker->numberBetween($min = 1, $max = 100),
        'deadline' => $faker->dateTimeBetween($startDate = 'now', $endDate = '+2 years', $timezone = 'Asia/Bishkek'),
        'active' => $faker->numberBetween($min = 0, $max = 1),
        'idFreelancer' =>  $faker->numberBetween($min = 0, $max = 1),
    ];
});