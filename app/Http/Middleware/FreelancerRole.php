<?php

namespace App\Http\Middleware;

use Closure;

class FreelancerRole
{
    /**
     * Handle an incoming request.
     *
     * @param  \Illuminate\Http\Request  $request
     * @param  \Closure  $next
     * @return mixed
     */
    public function handle($request, Closure $next)
   {
         $user = Auth::guard('api')->user();
         
        if($user->role != 1) {
           return response()->json(['error' => 'Вы не являетесь фрилансером для выполнения этого задание'], 401);
        }
        return $next($request);
    }
}