[%bs.raw {|require('./app.css')|}];

[@bs.module] external logo : string = "./logo.svg";

type article = {
  title: string
};

type response = {
  articles: list(article)
};

type state = {
  searchQuery: option(string),
  articles: list(article)
};

type action = 
  | SearchQueryUpdated(string)
  | ResponseReceived(response);

let initialState = {
  searchQuery: None,
  articles: []
}

let component = ReasonReact.reducerComponent("App");

let isBlank = (s: string) => (s |> String.length) < 3;

let articleDecoder = (json: Js.Json.t): article => 
  Json.Decode.{
    title: json |> field("title", string)
  };

let decodeJson = (json: Js.Json.t): response => 
  Json.Decode.{
    articles: json |> field("articles", list(articleDecoder))
  };

let make = (_children) => {
  ...component,
  initialState: () => initialState,
  reducer: (action, state) => 
    switch (action) {
    | SearchQueryUpdated(msg) when isBlank(msg) => ReasonReact.Update({...state, searchQuery: None})
    | SearchQueryUpdated(searchQuery) => ReasonReact.UpdateWithSideEffects(
        {...state, searchQuery: Some(searchQuery)}, 
        ({send}) => Fetch.fetch("https://newsapi.org/v2/everything?q=" ++ searchQuery ++ "&apiKey=a35ce68466704851bec15046387412f6") 
          |> Js.Promise.then_(Fetch.Response.json) 
          |> Js.Promise.then_(text => decodeJson(text) ->ResponseReceived |> send |> Js.Promise.resolve)
          |> Js.Promise.catch(_err => {
            Js.log(_err) |> Js.Promise.resolve
          })
          |> ignore
      )
    | ResponseReceived(response) => ReasonReact.Update({...state, articles: response.articles})
    },
  render: ({state, send}) =>
    <div className="App">
      <div className="App-header">
        <h1>(ReasonReact.string("ReasonML News Search"))</h1>
      </div>
      <div className="App-content">
        <input type_="text" onChange=(ev => (ReactEvent.Form.target(ev)##value) ->SearchQueryUpdated ->send) />
        <p>(ReasonReact.string(List.length(state.articles) |> string_of_int))</p>
        <ul>
          (ReasonReact.array(Array.of_list(List.map(a => <li>(ReasonReact.string(a.title))</li>, state.articles))))
        </ul>
      </div>
    </div>
};
