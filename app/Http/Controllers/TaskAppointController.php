<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use App\Task;
use App\User;
use App\FreelanceTask;
use Illuminate\Support\Facades\Auth;
use DB;

class TaskAppointController extends Controller
{
     public function store(Task $task, User $user)
    {
        $id = Auth::guard('api')->id();
        if($task->idCustomer == $id) {
            if($task->idFreelancer == 0)
            {
                DB::table('tasks')->where('id', $task->id)->update(['idFreelancer' => $user->id]);
    
    
                $ft = FreelanceTask::create();
                $ft->idFreelancer = $user->id;
                $ft->idTask = $task->id;
                $ft->save();
               
                return response()->json($ft, 201);
            }
            return response()->json(['error' => 'Фрилансер уже назначен'], 400);
        }
        return response()->json(['error' => 'Вы не являетесь владельцем задания'], 400);
    }
    public function update(Task $task)
    {
        $employer = Auth::guard('api')->user();
        $ft = DB::table('freelancer_tasks')->where('idTask', $task->id)->first();
        $freelancer = DB::table('users')->where('id', $ft->idFreelancer)->first();
        if($task->idCustomer == $employer->id) {
            DB::beginTransaction();
                if($ft->finished == 1)
               {
                   $ft->time_tested = 1;
                   if($employer->balance >= $task->price)
                   {
                       $balance_emp = $employer->balance - $task->price;
                       DB::table('users')->where('id', $employer->id)->update(['balance' => $balance_emp]);
                       $balance_freelancer = $freelancer->balance + $task->price;
                       DB::table('users')->where('id', $freelancer->id)->update(['balance' => $balance_freelancer]);
                       
                       DB::table('tasks')->where('id', $task->id)->update(['active' => 0]);
                       DB::table('freelancer_tasks')->where('idTask', $task->id)->update(['time_tested' => 1]);
                       $task->delete();
                       DB::commit();
                       return response()->json(['success' => 'Спасибо, что воспользовались нашими услугами, проверьте пожалуйста свой баланс'], 200);
                   }
               }
               DB::rollBack();
               return response()->json(['error' => 'У вас недостаточно денег для осуществления транзакции'], 400);
            }
            return response()->json(['error' => 'Вы не являетесь владельцем задания'], 400);
        }
    
}
