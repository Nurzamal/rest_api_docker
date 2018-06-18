<?php

use Illuminate\Database\Seeder;
use App\Task;

class TasksTableSeeder extends Seeder
{
    /**
     * Run the database seeds.
     *
     * @return void
     */
    public function run()
    {
        // Let's truncate our existing records to start from scratch.
        DB::table('tasks')->delete();

        $faker = \Faker\Factory::create();

        // And now, let's create a few articles in our database:
        for ($i = 0; $i < 50; $i++) {
            Task::create([
                'idCustomer' => $faker->numberBetween($min = 1, $max = 10),
                'title' => $faker->sentence,
                'description' => $faker->sentence,
                'price' => $faker->numberBetween($min = 10, $max = 1000),
                'deadline' => $faker->dateTimeBetween($startDate = 'now', $endDate = '+2 years', $timezone = 'Asia/Bishkek'),
            ]);
        }
    }
}
