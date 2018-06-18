<?php

namespace Tests\Feature;

use Tests\TestCase;
use Illuminate\Foundation\Testing\WithFaker;
use Illuminate\Foundation\Testing\RefreshDatabase;
use App\User;
use App\Task;
use App\FreelancerTask;

class TakingTaskTest extends TestCase
{
    /**
     * A basic test example.
     *
     * @return void
     */
    public function testTaskTakenCorrectly()
    {
        $user = factory(User::class)->create(['role' => 1]);
        $token = $user->generateToken();
        $headers = ['Authorization' => "Bearer $token"];

        $task = factory(Task::class)->create(['active' => 1, 'idFreelancer' => 0]);

        $payload = []; 

        $this->json('post', '/api/tasks/' . $task->id . '/take', $payload, $headers)
            ->assertStatus(201)
            ->assertJsonFragment([
                'success' => 'Succesfully requested'
               ]);
    }

    public function testTaskIsFinishedCorrectly() {
        $user = factory(User::class)->create(['role' => 1]);
        $token = $user->generateToken();
        $headers = ['Authorization' => "Bearer $token"];
        
        $task = factory(Task::class)->create(['idFreelancer' => $user->id, 'active' => 1]);
        
        $payload = [];

        $this->json('put', 'api/tasks/' . $task->id . '/finish', $payload, $headers)
            ->assertStatus(200)
            ->assertJsonFragment([
               'success' => 'You succefully finished the task, please wait for approval'
            ]);
    }
}