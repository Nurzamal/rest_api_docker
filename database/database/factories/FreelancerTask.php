<?php

use Faker\Generator as Faker;

$factory->define(App\FreelancerTask::class, function (Faker $faker) {
    return [
        'idFreelancer' => $faker->numberBetween($min = 1, $max = 10),
        'idTask' => $faker->numberBetween($min = 1, $max = 50),
    ];
});
