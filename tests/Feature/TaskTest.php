<?php

namespace Tests\Feature;

use Tests\TestCase;
use Illuminate\Foundation\Testing\WithFaker;
use Illuminate\Foundation\Testing\RefreshDatabase;
use App\Task;
use App\User;
use App\FreelancerTask;

class TaskTest extends TestCase
{
    public function testsTasksAreCreatedCorrectly()
    {
        $user = factory(User::class)->create(['role' => 0]);
        $token = $user->generateToken();
        $headers = ['Authorization' => "Bearer $token"];

        $payload = [
            "title" => "TEST",
            "description" => "IPSUM",
            "price" => 90,
            "deadline" => "1972-03-06",
        ];

        $this->json('POST', '/api/tasks', $payload, $headers)
            ->assertStatus(201)
            ->assertJsonFragment(['id' => 1, 'title' => 'TEST', 'description' => 'IPSUM']);
    }

    public function testsTasksAreUpdatedCorrectly()
    {
        $user = factory(User::class)->create(['role' => 0]);
        $token = $user->generateToken();
        $headers = ['Authorization' => "Bearer $token"];
        $task = factory(Task::class)->create([
            "title" => "TEST",
            "description" => "IPSUM",
            "price" => 1168825,
            "deadline" => "1972-03-06",
        ]);

        $payload = [
            'title' => 'TESTCHANGED',
        ];

        $response = $this->json('PUT', '/api/tasks/' . $task->id, $payload, $headers)
            ->assertStatus(200)
            ->assertJson([ 
                'id' => 1, 
                'title' => 'TESTCHANGED', 
            ]);
    }

    public function testsTasksAreDeletedCorrectly()
    {
        $user = factory(User::class)->create(['role' => 0]);
        $token = $user->generateToken();
        $headers = ['Authorization' => "Bearer $token"];
        $task = factory(Task::class)->create();

        $this->json('DELETE', '/api/tasks/' . $task->id, [], $headers)
            ->assertStatus(204);
    }

    public function testTasksAreListedCorrectly()
    {
        factory(Task::class)->create([
            'title' => 'First Article',
            'description' => 'First Body'
        ]);

        factory(Task::class)->create([
            'title' => 'Second Article',
            'description' => 'Second Body'
        ]);

        $user = factory(User::class)->create(['role' => 0]);
        $token = $user->generateToken();
        $headers = ['Authorization' => "Bearer $token"];

        $response = $this->json('GET', '/api/tasks', [], $headers)
            ->assertStatus(200)
            ->assertJsonFragment([
                'title' => 'First Article', 'description' => 'First Body' ,
                'title' => 'Second Article', 'description' => 'Second Body' 
            ])
            ->assertJsonStructure([
                '*' => ['id', 'idCustomer', 'title', 'description', 'price', 'deadline', 'active', 'idFreelancer', 'created_at', 'updated_at'],
            ]);
    }

    public function testFreelancerIsAssignedCorrectly()
    {
        $freelancer = factory(User::class)->create(['role' => 1]);

        $user = factory(User::class)->create(['role' => 0]);
        $token = $user->generateToken();
        $headers = ['Authorization' => "Bearer $token"];
        
        $task = factory(Task::class)->create([
            "idCustomer" => $user->id,
            "idFreelancer" => 0
        ]);

        $response = $this->json('POST', '/api/tasks/' . $task->id . '/' .$freelancer->id, [], $headers)
            ->assertStatus(201)
            ->assertJson([ 
                'id' => 1, 
                'idTask' => $task->id,
                'idFreelancer' => $freelancer->id 
            ]);
    }

     public function testApproval()
    {
        $user = factory(User::class)->create(['role' => 0]);
        $token = $user->generateToken();
        $headers = ['Authorization' => "Bearer $token"];

        $freelancer = factory(User::class)->create(['role' => 1]);

        $task = factory(Task::class)->create([
            "idCustomer" => $user->id,
            "idFreelancer" => $freelancer->id,
            'price' => 10
        ]);

        $ft = factory(FreelancerTask::class)->create([
            'idFreelancer' => $freelancer->id,
            'idTask' => $task->id,
            'finished' => 1
        ]);

        $response = $this->json('PUT', '/api/tasks/' . $task->id . '/approve', [], $headers)
            ->assertStatus(200)
            ->assertJsonFragment([ 
                'success' => 'Thanks for using our services. Check your balance'
            ]);
    }
}
