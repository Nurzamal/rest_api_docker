<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use App\Task;
use App\FreelanceTask;

class JobController extends Controller
{
    public function index()
    {
        return Task::all();
    }
    public function show(Task $task)
    {
        $ft = FreelancerTask::where('idTask', $task->id)->first();
        if($ft){
            return $ft;
        }
        return $task;
    }
    public function store(Request $request)
    {
        $user = Auth::guard('api')->user();
        if($user->balance >= $request->price) {
            $task = Task::create($request->all());
            $task->idCustomer = $user->id;
            $task->save();
    
            return response()->json($task, 201);
        }
        return response()->json(['error' => 'Уважаемый клиент у вас недостаточно денег на балансе,чтобы установить эту цену'], 400);
    }
    public function update(Request $request, Task $task)
    {
       
        $task->update($request->all());
        return response()->json($task, 200);
    }
    public function delete(Task $task)
    {
        $task->delete();
        return response()->json(null, 204);
    }
}