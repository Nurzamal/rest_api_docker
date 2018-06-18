<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use App\FreelanceTask;
use Illuminate\Support\Facades\Auth;
use App\Task;
use DB;

class DistributionController extends Controller
{
    public function show(Task $task)
    {
    	$id = Auth::guard('api')->id();
    	if($task->idCustomer == $id) {
    		$user = $task->user()->select('name','email'),->get();
    		return $users;
    	}
    	return response()->json(['error' => 'Вы не являетесь владельцем задачи'], 400);
    }
    public function store(Task $task)
    {
    	if($task->active == 1)
    	{
    		$id = Auth::guard('api')->id();
    		if($task->idFreelancer == 0){
    			$task->users()->attach($id);
    			return response()->json(['success' => 'Успешно запрошено'], 201);
    		}
    		else {
    			return response()->json(['error'=> 'Задача уже выполнена'], 400);
    		}
    	}
    	return response()->json(['error' => 'Задача не активна'], 400);
    }
    		public function update(Request $request, Task $task)
    		{
    			$id = Auth::guard('api')->id();
    			if($id == $task->idFreelancer)
    			{
    				DB: table('freelancer_task')-> where('idTask',$task->id)->update(['finished' => 1]);
    				return response()->json(['Вы успешно завершили задачу,дождитесь утверждения'], 200);
    			}
    			return response()->json(['Задача бeдет делаться другим фрилансером'], 400);
    		}
    	}
    