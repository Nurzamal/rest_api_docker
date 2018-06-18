<?php

namespace App\Http\Middleware;
use Illuminate\Support\Facades\Auth;

use Closure;

class ProcessingRole
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
        if($user->role != 0) {
           return response()->json(['error' => 'Вы не являетесь работодателем для выполнения этого задание'], 401);
        }
        return $next($request);
    }
}
