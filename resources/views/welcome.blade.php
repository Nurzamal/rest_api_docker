<!doctype html>
<html lang="{{ app()->getLocale() }}">
    <head>
        <meta charset="utf-8">
        <meta http-equiv="X-UA-Compatible" content="IE=edge">
        <meta name="viewport" content="width=device-width, initial-scale=1">

        <title>Laravel</title>

        <!-- Fonts -->
        <link href="https://fonts.googleapis.com/css?family=Raleway:100,600" rel="stylesheet" type="text/css">

        <!-- Styles -->
        <style>
            html, body {
                background-color: #fff;
                color: #636b6f;
                font-family: 'Raleway', sans-serif;
                font-weight: 100;
                height: 100vh;
                margin: 0;
            }
            .flex-center {
                align-items: center;
                display: flex;
                justify-content: center;
            }
            .position-ref {
                position: relative;
            }
            .top-right {
                position: absolute;
                right: 10px;
                top: 18px;
            }
            .content {
                text-align: center;
            }
            .content-left {
                margin-top: 2em;
                margin-left:2em;
            }
            .title {
                font-size: 84px;
            }
            .links > a {
                color: #636b6f;
                padding: 25px;
                font-size: 24px;
                font-weight: 600;
                letter-spacing: .1rem;
                text-decoration: none;
                text-transform: uppercase;
            }
            .m-b-md {
                margin-bottom: 30px;
            }
        </style>
    </head>
    <body>
        <div class="content-left links">
            @if( auth()->check() )
                
                    <a class="nav-link font-weight-bold" href="#">Hi {{ auth()->user()->name }}</a>
                    <a class="nav-link" href="/logout">Log Out</a>
                    @if( auth()->user()->role == 0)
                        <a class="nav-link" href="/addJob">Add task</a>
                    @endif
            @else
                    <a class="nav-link" href="/login">Log In</a>
                    <a class="nav-link" href="/register">Register</a>
            @endif
        </div>
        <div class="flex-center position-ref">
            <div class="content">
                <div class="title m-b-md">
                    Freelance 
                </div>

                <div class="jobs">
                <table>
                <tr>
                <th>Job title</th>
                <th>Description</th>
                <th>Payment</th>
                <th>Deadline</th>
                    <?php
                    foreach ($tasks as $task) {
                        echo "<tr><td>$task->title</td>";
                        echo "<td>$task->description</td>";
                        echo "<td>$task->price</td>";
                        echo "<td>$task->deadline</td></tr>";
                    }
                    ?>
                    </table>
                </div>
            </div>
        </div>
    </body>
</html>
